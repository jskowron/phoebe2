#ifdef HAVE_CONFIG_H
#  include "phoebe_gui_build_config.h"
#endif

#include <stdlib.h>
#include <string.h>
#include <phoebe/phoebe.h>

#include "phoebe_gui_accessories.h"
#include "phoebe_gui_base.h"
#include "phoebe_gui_callbacks.h"
#include "phoebe_gui_error_handling.h"
#include "phoebe_gui_main.h"
#include "phoebe_gui_treeviews.h"
#include "phoebe_gui_types.h"

#ifdef __MINGW32__
#include <glade/glade-build.h>
#include <libgen.h>

gchar *PROGRAM_NAME = "";

// GtkFileChooserButton is not defined in libglade for MinGW/MSYS, so it has to be defined here
GtkWidget *gui_GtkFileChooserButton(GladeXML *xml, GType widget_type, GladeWidgetInfo *info)
{
	gint width_chars = 0;
	GtkFileChooserAction action = GTK_FILE_CHOOSER_ACTION_OPEN;
	gchar *title = NULL;
	gint i;

	for (i = 0; i < info->n_properties; i++) {
		if (!strcmp (info->properties[i].name, "width_chars")) {
			width_chars = atoi(info->properties[i].value);
		}
		else if (!strcmp (info->properties[i].name, "action")) {
			if (!strcmp (info->properties[i].value, "GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER"))
				action = GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER;
		}
		else if (!strcmp (info->properties[i].name, "title")) {
			title = info->properties[i].value;
		}
	}
	GtkWidget *filechooserbutton = gtk_file_chooser_button_new (title, action);
	gtk_widget_show(filechooserbutton);
	if (width_chars > 0)
		gtk_file_chooser_button_set_width_chars(GTK_FILE_CHOOSER_BUTTON(filechooserbutton), width_chars);

	return filechooserbutton;
}

// GtkAboutDialog is not defined in libglade for MinGW/MSYS, so it has to be defined here
GtkWidget *gui_GtkAboutDialog(GladeXML *xml, GType widget_type, GladeWidgetInfo *info)
{
	gchar *program_name = "";
	gchar *version = "";
	gchar *comments = "";
	gchar *license = "";
	gchar *website = "";
	gchar *website_label = "";
	gchar **authors = malloc(30 * sizeof(gchar *));
	gchar **documenters = malloc(30 * sizeof(gchar *));
	gint i;

	for (i = 0; i < info->n_properties; i++) {
		if (!strcmp (info->properties[i].name, "program_name")) {
			program_name = info->properties[i].value;
		}
		else if (!strcmp (info->properties[i].name, "version")) {
			version = info->properties[i].value;
		}
		else if (!strcmp (info->properties[i].name, "comments")) {
			comments = info->properties[i].value;
		}
		else if (!strcmp (info->properties[i].name, "license")) {
			license = info->properties[i].value;
		}
		else if (!strcmp (info->properties[i].name, "website")) {
			website = info->properties[i].value;
		}
		else if (!strcmp (info->properties[i].name, "website_label")) {
			website_label = info->properties[i].value;
		}
		else if (!strcmp (info->properties[i].name, "authors")) {
			int j = 0;
			authors[j++] = strtok (info->properties[i].value, "\n");
			while ((authors[j++] = strtok (NULL, ",")));
		}
		else if (!strcmp (info->properties[i].name, "documenters")) {
			int j = 0;
			documenters[j++] = strtok (info->properties[i].value, "\n");
			while ((documenters[j++] = strtok (NULL, ",")));
		}
	}
	GtkWidget *aboutdialog = gtk_about_dialog_new ();
	gtk_about_dialog_set_program_name(GTK_ABOUT_DIALOG(aboutdialog), (PROGRAM_NAME) ? PROGRAM_NAME : program_name);
	gtk_about_dialog_set_version(GTK_ABOUT_DIALOG(aboutdialog), version);
	gtk_about_dialog_set_comments(GTK_ABOUT_DIALOG(aboutdialog), comments);
	gtk_about_dialog_set_license(GTK_ABOUT_DIALOG(aboutdialog), license);
	gtk_about_dialog_set_website(GTK_ABOUT_DIALOG(aboutdialog), website);
	gtk_about_dialog_set_website_label(GTK_ABOUT_DIALOG(aboutdialog), website_label);
	gtk_about_dialog_set_authors(GTK_ABOUT_DIALOG(aboutdialog), (const gchar **)authors);
	gtk_about_dialog_set_documenters(GTK_ABOUT_DIALOG(aboutdialog), (const gchar **)documenters);

	free(documenters);

	return aboutdialog;
}
#endif

int parse_startup_line (int argc, char *argv[])
{
	/*
	 * This function parses the command line and looks for known switches.
	 */

	int i, status;

	for (i = 1; i < argc; i++) {
		if ( (strcmp (argv[i],  "-h"   ) == 0) ||
		     (strcmp (argv[i],  "-?"   ) == 0) ||
		     (strcmp (argv[i], "--help") == 0) ) {
			printf ("\n%s command line arguments: [-hv] [parameter_file]\n\n", PHOEBE_GUI_RELEASE_NAME);
			printf ("  -h, --help, -?      ..  this help screen\n");
			printf ("  -v, --version       ..  display PHOEBE version and exit\n");
			printf ("\n");
			phoebe_quit ();
		}

		if ( (strcmp (argv[i],  "-v"      ) == 0) ||
			 (strcmp (argv[i], "--version") == 0) ) {
			printf ("\n%s, %s\n", PHOEBE_GUI_RELEASE_NAME, PHOEBE_GUI_RELEASE_DATE);
			printf ("  Send comments and/or requests to phoebe-discuss@lists.sourceforge.net\n\n");
			phoebe_quit ();
		}

		if ( argv[i][0] != '-' ) {
			/*
			 * This means that the command line argument doesn't contain '-';
			 * thus it is a parameter file.
			 */

			status = phoebe_open_parameter_file (argv[i]);
			if (status != SUCCESS)
				phoebe_gui_output ("%s", phoebe_error (status));
			else {
				gui_reinit_treeviews ();
				gui_set_values_to_widgets ();
				PHOEBE_FILEFLAG = TRUE;
				PHOEBE_FILENAME = strdup (argv[i]);
			}
		}
	}

	return SUCCESS;
}

int main (int argc, char *argv[])
{
	int status;
	bool configswitch = FALSE;

	gtk_set_locale ();
	gtk_init (&argc, &argv);
	glade_init ();
#ifdef __MINGW32__
	PROGRAM_NAME = basename(argv[0]);
	char *exe = strstr(PROGRAM_NAME, ".exe");
	if (exe)
		*exe = '\0';
	glade_register_widget (GTK_TYPE_FILE_CHOOSER_BUTTON, gui_GtkFileChooserButton, NULL, NULL);
	glade_register_widget (GTK_TYPE_ABOUT_DIALOG, gui_GtkAboutDialog, NULL, NULL);
#endif

	status = phoebe_init ();
	if (status != SUCCESS) {
		printf ("%s", phoebe_gui_error (status));
		exit (0);
	}

	/* Add all GUI-related options here: */
	phoebe_config_entry_add (TYPE_BOOL, "GUI_CONFIRM_ON_OVERWRITE", TRUE);
	phoebe_config_entry_add (TYPE_BOOL, "GUI_BEEP_AFTER_PLOT_AND_FIT", FALSE);

	status = phoebe_configure ();
	if (status == ERROR_PHOEBE_CONFIG_SUPPORTED_FILE ||
		status == ERROR_PHOEBE_CONFIG_LEGACY_FILE    ||
		status == ERROR_PHOEBE_CONFIG_NOT_FOUND)
		/* This happens when the configuration file is imported from a recent
		 * version (that is fully supported) or from a pre-0.30 (legacy)
		 * version. In these cases we should pop up the configuration screen
		 * for the user to review the settings.
		 */
		configswitch = TRUE;

	phoebe_gui_init ();

	parse_startup_line (argc, argv);

	if (status == ERROR_PHOEBE_CONFIG_NOT_FOUND)
		gui_notice ("Welcome to PHOEBE!", "PHOEBE will create a configuration directory and take you to the Settings window.");
	if (status == ERROR_PHOEBE_CONFIG_LEGACY_FILE)
		gui_notice ("Importing legacy configuration file", "PHOEBE imported a legacy (pre-0.30) configuration file. Please review your settings and click on Save to store them permanently.");
	if (status == ERROR_PHOEBE_CONFIG_SUPPORTED_FILE)
		gui_notice ("Importing recent configuration file", "PHOEBE imported your previous configuration file. Please review your settings and click on Save to store them permanently.");

	if (configswitch)
		gui_show_configuration_dialog();

	gtk_main ();


	phoebe_gui_quit ();

	phoebe_quit ();

	return SUCCESS;
}
