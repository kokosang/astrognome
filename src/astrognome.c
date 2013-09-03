#include <glib.h>
#include <gtk/gtk.h>

#include "../swe-glib/src/swe-glib.h"

#define EPHEDIR "/home/polesz/Projektek/c/astrognome/swe/data"
#define UI_FILE "/home/polesz/Projektek/c/astrognome/src/astrognome.ui"

GtkBuilder *builder;

const char *moonStateName[] = {
    "New Moon",
    "Waxing Crescent Moon",
    "Waxing Half Moon",
    "Waxing Gibbous Moon",
    "Full Moon",
    "Waning Gibbous Moon",
    "Waning Half Moon",
    "Waning Crescent Moon",
    "Dark Moon"
};

struct print_data {
    GString *line;
    GswePlanet planet;
};

void
print_house_cusp(GsweHouseData *house_data, GString *line)
{
    g_string_append_printf(line, "House %d: %s (%f)\n", house_data->house, house_data->sign->name, house_data->cusp_position);
}

void
print_aspects(GsweAspectData *aspect_data, struct print_data *data)
{
    GswePlanetData *other_planet = (aspect_data->planet1->planet_id == data->planet) ? aspect_data->planet2 : aspect_data->planet1;

    if (aspect_data->aspect != GSWE_ASPECT_NONE) {
        g_string_append_printf(data->line, "        %s: %s (±%f%%)\n", other_planet->planet_info->name, aspect_data->aspect_info->name, aspect_data->difference);
    }
}

void
print_mirrorpoints(GsweMirrorData *mirror_data, struct print_data *data)
{
    GswePlanetData *other_planet = (mirror_data->planet1->planet_id == data->planet) ? mirror_data->planet2 : mirror_data->planet1;

    if (mirror_data->mirror != GSWE_MIRROR_NONE) {
        g_string_append_printf(data->line, "        %s: %s (±%f°)\n", other_planet->planet_info->name, mirror_data->mirror_info->name, mirror_data->difference);
    }
}

void
action_about_activate_cb(GtkAction *action, gpointer user_data)
{
    GtkDialog *dialog_about = GTK_DIALOG(gtk_builder_get_object(builder, "dialog_about"));

    gtk_dialog_run(dialog_about);

    gtk_widget_hide(GTK_WIDGET(dialog_about));
}

void
create_new_chart(void)
{
    guint year,
         month,
         day,
         hour,
         minute,
         second;
    gdouble longitude,
            latitude,
            altitude,
            time_zone;
    GsweTimestamp *timestamp;
    GsweMoment *moment;
    GtkTextBuffer *buffer;
    GtkWidget *view;
    GtkWidget *label;
    GswePlanetData *planet_data;
    GsweMoonPhaseData *moon_phase;
    GList *aspects,
          *mirrorpoints;
    GtkWidget *scrolled_window;
    struct print_data data;

    gtk_calendar_get_date(GTK_CALENDAR(gtk_builder_get_object(builder, "calendar_new_date")), &year, &month, &day);
    month++;
    hour = (guint)gtk_spin_button_get_value(GTK_SPIN_BUTTON(gtk_builder_get_object(builder, "spin_new_hour")));
    minute = (guint)gtk_spin_button_get_value(GTK_SPIN_BUTTON(gtk_builder_get_object(builder, "spin_new_minute")));
    second = (guint)gtk_spin_button_get_value(GTK_SPIN_BUTTON(gtk_builder_get_object(builder, "spin_new_second")));
    longitude = gtk_spin_button_get_value(GTK_SPIN_BUTTON(gtk_builder_get_object(builder, "spin_new_longitude")));
    latitude = gtk_spin_button_get_value(GTK_SPIN_BUTTON(gtk_builder_get_object(builder, "spin_new_latitude")));
    // TODO: these should be real values (at least time_zone)!
    time_zone = 1.0;
    altitude = 200.0;

    timestamp = gswe_timestamp_new_from_gregorian_full(year, month, day, hour, minute, second, 0, time_zone);
    // TODO: Make house system configurable
    moment = gswe_moment_new_full(timestamp, longitude, latitude, altitude, GSWE_HOUSE_SYSTEM_PLACIDUS);
    // TODO: Make planet list customizable
    gswe_moment_add_all_planets(moment);

    data.line = g_string_sized_new(300);

    g_string_append_printf(data.line, "Date: %02d.%02d.%d at %02d:%02d:%02d, at %f, %f\n", year, month, day, hour, minute, second, longitude, latitude);
    g_string_append_printf(data.line, "\nHOUSES\n======\n\n");
    g_list_foreach(gswe_moment_get_house_cusps(moment), (GFunc)print_house_cusp, data.line);
    g_string_append_printf(data.line, "\nPLANETS AND POINTS\n==================\n\n");

    // Ascendent
    planet_data = gswe_moment_get_planet(moment, GSWE_PLANET_ASCENDENT);
    data.planet = GSWE_PLANET_ASCENDENT;
    g_string_append_printf(data.line, "%s: %s (%f)\n", planet_data->planet_info->name, planet_data->sign->name, planet_data->position);

    aspects = gswe_moment_get_planet_aspects(moment, GSWE_PLANET_ASCENDENT);
    g_string_append_printf(data.line, "    Aspects:\n");
    g_list_foreach(aspects, (GFunc)print_aspects, &data);
    g_list_free(aspects);

    mirrorpoints = gswe_moment_get_all_planet_mirrorpoints(moment, GSWE_PLANET_ASCENDENT);
    g_string_append_printf(data.line, "    Antiscia:\n");
    g_list_foreach(mirrorpoints, (GFunc)print_mirrorpoints, &data);
    g_list_free(mirrorpoints);

    // Midheaven
    planet_data = gswe_moment_get_planet(moment, GSWE_PLANET_MC);
    data.planet = GSWE_PLANET_MC;
    g_string_append_printf(data.line, "%s: %s (%f)\n", planet_data->planet_info->name, planet_data->sign->name, planet_data->position);

    aspects = gswe_moment_get_planet_aspects(moment, GSWE_PLANET_MC);
    g_string_append_printf(data.line, "    Aspects:\n");
    g_list_foreach(aspects, (GFunc)print_aspects, &data);
    g_list_free(aspects);

    mirrorpoints = gswe_moment_get_all_planet_mirrorpoints(moment, GSWE_PLANET_MC);
    g_string_append_printf(data.line, "    Antiscia:\n");
    g_list_foreach(mirrorpoints, (GFunc)print_mirrorpoints, &data);
    g_list_free(mirrorpoints);

    // Vertex
    planet_data = gswe_moment_get_planet(moment, GSWE_PLANET_VERTEX);
    data.planet = GSWE_PLANET_VERTEX;
    g_string_append_printf(data.line, "%s: %s (%f)\n", planet_data->planet_info->name, planet_data->sign->name, planet_data->position);

    aspects = gswe_moment_get_planet_aspects(moment, GSWE_PLANET_VERTEX);
    g_string_append_printf(data.line, "    Aspects:\n");
    g_list_foreach(aspects, (GFunc)print_aspects, &data);
    g_list_free(aspects);

    mirrorpoints = gswe_moment_get_all_planet_mirrorpoints(moment, GSWE_PLANET_VERTEX);
    g_string_append_printf(data.line, "    Antiscia:\n");
    g_list_foreach(mirrorpoints, (GFunc)print_mirrorpoints, &data);
    g_list_free(mirrorpoints);

    // Sun
    planet_data = gswe_moment_get_planet(moment, GSWE_PLANET_SUN);
    data.planet = GSWE_PLANET_SUN;
    g_string_append_printf(data.line, "%s: %s, House: %d (%f%s)\n", planet_data->planet_info->name, planet_data->sign->name, planet_data->house, planet_data->position, (planet_data->retrograde) ? ", retrograde" : "");

    aspects = gswe_moment_get_planet_aspects(moment, GSWE_PLANET_SUN);
    g_string_append_printf(data.line, "    Aspects:\n");
    g_list_foreach(aspects, (GFunc)print_aspects, &data);
    g_list_free(aspects);

    mirrorpoints = gswe_moment_get_all_planet_mirrorpoints(moment, GSWE_PLANET_SUN);
    g_string_append_printf(data.line, "    Antiscia:\n");
    g_list_foreach(mirrorpoints, (GFunc)print_mirrorpoints, &data);
    g_list_free(mirrorpoints);

    // Moon
    moon_phase = gswe_moment_get_moon_phase(moment);
    planet_data = gswe_moment_get_planet(moment, GSWE_PLANET_MOON);
    data.planet = GSWE_PLANET_MOON;
    g_string_append_printf(data.line, "%s: %s (%.2f%% visibility), %s, House: %d (%f%s)\n", planet_data->planet_info->name, moonStateName[moon_phase->phase], moon_phase->illumination, planet_data->sign->name, planet_data->house, planet_data->position, (planet_data->retrograde) ? ", retrograde" : "");

    aspects = gswe_moment_get_planet_aspects(moment, GSWE_PLANET_MOON);
    g_string_append_printf(data.line, "    Aspects:\n");
    g_list_foreach(aspects, (GFunc)print_aspects, &data);
    g_list_free(aspects);

    mirrorpoints = gswe_moment_get_all_planet_mirrorpoints(moment, GSWE_PLANET_MOON);
    g_string_append_printf(data.line, "    Antiscia:\n");
    g_list_foreach(mirrorpoints, (GFunc)print_mirrorpoints, &data);
    g_list_free(mirrorpoints);

    // Mercury
    planet_data = gswe_moment_get_planet(moment, GSWE_PLANET_MERCURY);
    data.planet = GSWE_PLANET_MERCURY;
    g_string_append_printf(data.line, "%s: %s, House: %d (%f%s)\n", planet_data->planet_info->name, planet_data->sign->name, planet_data->house, planet_data->position, (planet_data->retrograde) ? ", retrograde" : "");

    aspects = gswe_moment_get_planet_aspects(moment, GSWE_PLANET_MERCURY);
    g_string_append_printf(data.line, "    Aspects:\n");
    g_list_foreach(aspects, (GFunc)print_aspects, &data);
    g_list_free(aspects);

    mirrorpoints = gswe_moment_get_all_planet_mirrorpoints(moment, GSWE_PLANET_MERCURY);
    g_string_append_printf(data.line, "    Antiscia:\n");
    g_list_foreach(mirrorpoints, (GFunc)print_mirrorpoints, &data);
    g_list_free(mirrorpoints);

    // Venus
    planet_data = gswe_moment_get_planet(moment, GSWE_PLANET_VENUS);
    data.planet = GSWE_PLANET_VENUS;
    g_string_append_printf(data.line, "%s: %s, House: %d (%f%s)\n", planet_data->planet_info->name, planet_data->sign->name, planet_data->house, planet_data->position, (planet_data->retrograde) ? ", retrograde" : "");

    aspects = gswe_moment_get_planet_aspects(moment, GSWE_PLANET_VENUS);
    g_string_append_printf(data.line, "    Aspects:\n");
    g_list_foreach(aspects, (GFunc)print_aspects, &data);
    g_list_free(aspects);

    mirrorpoints = gswe_moment_get_all_planet_mirrorpoints(moment, GSWE_PLANET_VENUS);
    g_string_append_printf(data.line, "    Antiscia:\n");
    g_list_foreach(mirrorpoints, (GFunc)print_mirrorpoints, &data);
    g_list_free(mirrorpoints);

    // Mars
    planet_data = gswe_moment_get_planet(moment, GSWE_PLANET_MARS);
    data.planet = GSWE_PLANET_MARS;
    g_string_append_printf(data.line, "%s: %s, House: %d (%f%s)\n", planet_data->planet_info->name, planet_data->sign->name, planet_data->house, planet_data->position, (planet_data->retrograde) ? ", retrograde" : "");

    aspects = gswe_moment_get_planet_aspects(moment, GSWE_PLANET_MARS);
    g_string_append_printf(data.line, "    Aspects:\n");
    g_list_foreach(aspects, (GFunc)print_aspects, &data);
    g_list_free(aspects);

    mirrorpoints = gswe_moment_get_all_planet_mirrorpoints(moment, GSWE_PLANET_MARS);
    g_string_append_printf(data.line, "    Antiscia:\n");
    g_list_foreach(mirrorpoints, (GFunc)print_mirrorpoints, &data);
    g_list_free(mirrorpoints);

    // Jupiter
    planet_data = gswe_moment_get_planet(moment, GSWE_PLANET_JUPITER);
    data.planet = GSWE_PLANET_JUPITER;
    g_string_append_printf(data.line, "%s: %s, House: %d (%f%s)\n", planet_data->planet_info->name, planet_data->sign->name, planet_data->house, planet_data->position, (planet_data->retrograde) ? ", retrograde" : "");

    aspects = gswe_moment_get_planet_aspects(moment, GSWE_PLANET_JUPITER);
    g_string_append_printf(data.line, "    Aspects:\n");
    g_list_foreach(aspects, (GFunc)print_aspects, &data);
    g_list_free(aspects);

    mirrorpoints = gswe_moment_get_all_planet_mirrorpoints(moment, GSWE_PLANET_JUPITER);
    g_string_append_printf(data.line, "    Antiscia:\n");
    g_list_foreach(mirrorpoints, (GFunc)print_mirrorpoints, &data);
    g_list_free(mirrorpoints);

    // Saturn
    planet_data = gswe_moment_get_planet(moment, GSWE_PLANET_SATURN);
    data.planet = GSWE_PLANET_SATURN;
    g_string_append_printf(data.line, "%s: %s, House: %d (%f%s)\n", planet_data->planet_info->name, planet_data->sign->name, planet_data->house, planet_data->position, (planet_data->retrograde) ? ", retrograde" : "");

    aspects = gswe_moment_get_planet_aspects(moment, GSWE_PLANET_SATURN);
    g_string_append_printf(data.line, "    Aspects:\n");
    g_list_foreach(aspects, (GFunc)print_aspects, &data);
    g_list_free(aspects);

    mirrorpoints = gswe_moment_get_all_planet_mirrorpoints(moment, GSWE_PLANET_SATURN);
    g_string_append_printf(data.line, "    Antiscia:\n");
    g_list_foreach(mirrorpoints, (GFunc)print_mirrorpoints, &data);
    g_list_free(mirrorpoints);

    // Uranus
    planet_data = gswe_moment_get_planet(moment, GSWE_PLANET_URANUS);
    data.planet = GSWE_PLANET_URANUS;
    g_string_append_printf(data.line, "%s: %s, House: %d (%f%s)\n", planet_data->planet_info->name, planet_data->sign->name, planet_data->house, planet_data->position, (planet_data->retrograde) ? ", retrograde" : "");

    aspects = gswe_moment_get_planet_aspects(moment, GSWE_PLANET_URANUS);
    g_string_append_printf(data.line, "    Aspects:\n");
    g_list_foreach(aspects, (GFunc)print_aspects, &data);
    g_list_free(aspects);

    mirrorpoints = gswe_moment_get_all_planet_mirrorpoints(moment, GSWE_PLANET_URANUS);
    g_string_append_printf(data.line, "    Antiscia:\n");
    g_list_foreach(mirrorpoints, (GFunc)print_mirrorpoints, &data);
    g_list_free(mirrorpoints);

    // Neptune
    planet_data = gswe_moment_get_planet(moment, GSWE_PLANET_NEPTUNE);
    data.planet = GSWE_PLANET_NEPTUNE;
    g_string_append_printf(data.line, "%s: %s, House: %d (%f%s)\n", planet_data->planet_info->name, planet_data->sign->name, planet_data->house, planet_data->position, (planet_data->retrograde) ? ", retrograde" : "");

    aspects = gswe_moment_get_planet_aspects(moment, GSWE_PLANET_NEPTUNE);
    g_string_append_printf(data.line, "    Aspects:\n");
    g_list_foreach(aspects, (GFunc)print_aspects, &data);
    g_list_free(aspects);

    mirrorpoints = gswe_moment_get_all_planet_mirrorpoints(moment, GSWE_PLANET_NEPTUNE);
    g_string_append_printf(data.line, "    Antiscia:\n");
    g_list_foreach(mirrorpoints, (GFunc)print_mirrorpoints, &data);
    g_list_free(mirrorpoints);

    // Pluto
    planet_data = gswe_moment_get_planet(moment, GSWE_PLANET_PLUTO);
    data.planet = GSWE_PLANET_PLUTO;
    g_string_append_printf(data.line, "%s: %s, House: %d (%f%s)\n", planet_data->planet_info->name, planet_data->sign->name, planet_data->house, planet_data->position, (planet_data->retrograde) ? ", retrograde" : "");

    aspects = gswe_moment_get_planet_aspects(moment, GSWE_PLANET_PLUTO);
    g_string_append_printf(data.line, "    Aspects:\n");
    g_list_foreach(aspects, (GFunc)print_aspects, &data);
    g_list_free(aspects);

    mirrorpoints = gswe_moment_get_all_planet_mirrorpoints(moment, GSWE_PLANET_PLUTO);
    g_string_append_printf(data.line, "    Antiscia:\n");
    g_list_foreach(mirrorpoints, (GFunc)print_mirrorpoints, &data);
    g_list_free(mirrorpoints);

    // Mean node
    planet_data = gswe_moment_get_planet(moment, GSWE_PLANET_MOON_NODE);
    data.planet = GSWE_PLANET_MOON_NODE;
    g_string_append_printf(data.line, "%s: %s, House: %d (%f%s)\n", planet_data->planet_info->name, planet_data->sign->name, planet_data->house, planet_data->position, (planet_data->retrograde) ? ", retrograde" : "");

    aspects = gswe_moment_get_planet_aspects(moment, GSWE_PLANET_MOON_NODE);
    g_string_append_printf(data.line, "    Aspects:\n");
    g_list_foreach(aspects, (GFunc)print_aspects, &data);
    g_list_free(aspects);

    mirrorpoints = gswe_moment_get_all_planet_mirrorpoints(moment, GSWE_PLANET_MOON_NODE);
    g_string_append_printf(data.line, "    Antiscia:\n");
    g_list_foreach(mirrorpoints, (GFunc)print_mirrorpoints, &data);
    g_list_free(mirrorpoints);

    // Mean apogee
    planet_data = gswe_moment_get_planet(moment, GSWE_PLANET_MOON_APOGEE);
    data.planet = GSWE_PLANET_MOON_APOGEE;
    g_string_append_printf(data.line, "%s: %s, House: %d (%f%s)\n", planet_data->planet_info->name, planet_data->sign->name, planet_data->house, planet_data->position, (planet_data->retrograde) ? ", retrograde" : "");

    aspects = gswe_moment_get_planet_aspects(moment, GSWE_PLANET_MOON_APOGEE);
    g_string_append_printf(data.line, "    Aspects:\n");
    g_list_foreach(aspects, (GFunc)print_aspects, &data);
    g_list_free(aspects);

    mirrorpoints = gswe_moment_get_all_planet_mirrorpoints(moment, GSWE_PLANET_MOON_APOGEE);
    g_string_append_printf(data.line, "    Antiscia:\n");
    g_list_foreach(mirrorpoints, (GFunc)print_mirrorpoints, &data);
    g_list_free(mirrorpoints);

    // Chiron
    planet_data = gswe_moment_get_planet(moment, GSWE_PLANET_CHIRON);
    data.planet = GSWE_PLANET_CHIRON;
    g_string_append_printf(data.line, "%s: %s, House: %d (%f%s)\n", planet_data->planet_info->name, planet_data->sign->name, planet_data->house, planet_data->position, (planet_data->retrograde) ? ", retrograde" : "");

    aspects = gswe_moment_get_planet_aspects(moment, GSWE_PLANET_CHIRON);
    g_string_append_printf(data.line, "    Aspects:\n");
    g_list_foreach(aspects, (GFunc)print_aspects, &data);
    g_list_free(aspects);

    mirrorpoints = gswe_moment_get_all_planet_mirrorpoints(moment, GSWE_PLANET_CHIRON);
    g_string_append_printf(data.line, "    Antiscia:\n");
    g_list_foreach(mirrorpoints, (GFunc)print_mirrorpoints, &data);
    g_list_free(mirrorpoints);

    // Ceres
    planet_data = gswe_moment_get_planet(moment, GSWE_PLANET_CERES);
    data.planet = GSWE_PLANET_CERES;
    g_string_append_printf(data.line, "%s: %s, House: %d (%f%s)\n", planet_data->planet_info->name, planet_data->sign->name, planet_data->house, planet_data->position, (planet_data->retrograde) ? ", retrograde" : "");

    aspects = gswe_moment_get_planet_aspects(moment, GSWE_PLANET_CERES);
    g_string_append_printf(data.line, "    Aspects:\n");
    g_list_foreach(aspects, (GFunc)print_aspects, &data);
    g_list_free(aspects);

    mirrorpoints = gswe_moment_get_all_planet_mirrorpoints(moment, GSWE_PLANET_CERES);
    g_string_append_printf(data.line, "    Antiscia:\n");
    g_list_foreach(mirrorpoints, (GFunc)print_mirrorpoints, &data);
    g_list_free(mirrorpoints);

    // Pallas
    planet_data = gswe_moment_get_planet(moment, GSWE_PLANET_PALLAS);
    data.planet = GSWE_PLANET_PALLAS;
    g_string_append_printf(data.line, "%s: %s, House: %d (%f%s)\n", planet_data->planet_info->name, planet_data->sign->name, planet_data->house, planet_data->position, (planet_data->retrograde) ? ", retrograde" : "");

    aspects = gswe_moment_get_planet_aspects(moment, GSWE_PLANET_PALLAS);
    g_string_append_printf(data.line, "    Aspects:\n");
    g_list_foreach(aspects, (GFunc)print_aspects, &data);
    g_list_free(aspects);

    mirrorpoints = gswe_moment_get_all_planet_mirrorpoints(moment, GSWE_PLANET_PALLAS);
    g_string_append_printf(data.line, "    Antiscia:\n");
    g_list_foreach(mirrorpoints, (GFunc)print_mirrorpoints, &data);
    g_list_free(mirrorpoints);

    // Juno
    planet_data = gswe_moment_get_planet(moment, GSWE_PLANET_JUNO);
    data.planet = GSWE_PLANET_JUNO;
    g_string_append_printf(data.line, "%s: %s, House: %d (%f%s)\n", planet_data->planet_info->name, planet_data->sign->name, planet_data->house, planet_data->position, (planet_data->retrograde) ? ", retrograde" : "");

    aspects = gswe_moment_get_planet_aspects(moment, GSWE_PLANET_JUNO);
    g_string_append_printf(data.line, "    Aspects:\n");
    g_list_foreach(aspects, (GFunc)print_aspects, &data);
    g_list_free(aspects);

    mirrorpoints = gswe_moment_get_all_planet_mirrorpoints(moment, GSWE_PLANET_JUNO);
    g_string_append_printf(data.line, "    Antiscia:\n");
    g_list_foreach(mirrorpoints, (GFunc)print_mirrorpoints, &data);
    g_list_free(mirrorpoints);

    // Vesta
    planet_data = gswe_moment_get_planet(moment, GSWE_PLANET_VESTA);
    data.planet = GSWE_PLANET_VESTA;
    g_string_append_printf(data.line, "%s: %s, House: %d (%f%s)\n", planet_data->planet_info->name, planet_data->sign->name, planet_data->house, planet_data->position, (planet_data->retrograde) ? ", retrograde" : "");

    aspects = gswe_moment_get_planet_aspects(moment, GSWE_PLANET_VESTA);
    g_string_append_printf(data.line, "    Aspects:\n");
    g_list_foreach(aspects, (GFunc)print_aspects, &data);
    g_list_free(aspects);

    mirrorpoints = gswe_moment_get_all_planet_mirrorpoints(moment, GSWE_PLANET_VESTA);
    g_string_append_printf(data.line, "    Antiscia:\n");
    g_list_foreach(mirrorpoints, (GFunc)print_mirrorpoints, &data);
    g_list_free(mirrorpoints);

    g_string_append_printf(data.line, "\nELEMENTS\n========\n\n");

    g_string_append_printf(data.line, "Fire: %d\n", gswe_moment_get_element_points(moment, GSWE_ELEMENT_FIRE));
    g_string_append_printf(data.line, "Earth: %d\n", gswe_moment_get_element_points(moment, GSWE_ELEMENT_EARTH));
    g_string_append_printf(data.line, "Air: %d\n", gswe_moment_get_element_points(moment, GSWE_ELEMENT_AIR));
    g_string_append_printf(data.line, "Water: %d\n", gswe_moment_get_element_points(moment, GSWE_ELEMENT_WATER));

    g_string_append_printf(data.line, "\nQUALITIES\n=========\n\n");

    g_string_append_printf(data.line, "Cardinal: %d\n", gswe_moment_get_quality_points(moment, GSWE_QUALITY_CARDINAL));
    g_string_append_printf(data.line, "Fix: %d\n", gswe_moment_get_quality_points(moment, GSWE_QUALITY_FIX));
    g_string_append_printf(data.line, "Mutable: %d\n", gswe_moment_get_quality_points(moment, GSWE_QUALITY_MUTABLE));


    buffer = gtk_text_buffer_new(NULL);
    gtk_text_buffer_insert_at_cursor(buffer, data.line->str, data.line->len);
    view = gtk_text_view_new_with_buffer(buffer);
    scrolled_window = gtk_scrolled_window_new(NULL, NULL);
    gtk_container_add(GTK_CONTAINER(scrolled_window), view);

    label = gtk_label_new(gtk_entry_get_text(GTK_ENTRY(gtk_builder_get_object(builder, "entry_new_name"))));
    gtk_notebook_append_page(GTK_NOTEBOOK(gtk_builder_get_object(builder, "notebook_chartlist")), scrolled_window, label);
    gtk_widget_show_all(scrolled_window);
}

void
action_new_activate_cb(GtkAction *action, gpointer user_data)
{
    gboolean cont = TRUE;
    GtkDialog *dialog_new = GTK_DIALOG(gtk_builder_get_object(builder, "dialog_new"));

    gtk_window_set_title(GTK_WINDOW(dialog_new), "New chart");

    while (cont) {
        gint response = gtk_dialog_run(dialog_new);

        switch (response) {
            case GTK_RESPONSE_OK:
                g_print("OK\n");

                {
                    gint name_length = gtk_entry_get_text_length(GTK_ENTRY(gtk_builder_get_object(builder, "entry_new_name")));
                    gboolean lat_selected = (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(gtk_builder_get_object(builder, "radio_new_north"))) || gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(gtk_builder_get_object(builder, "radio_new_south"))));
                    gboolean long_selected = (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(gtk_builder_get_object(builder, "radio_new_east"))) || gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(gtk_builder_get_object(builder, "radio_new_west"))));

                    if ((name_length > 0) && (lat_selected == TRUE) && (long_selected == TRUE)) {
                        cont = FALSE;

                        create_new_chart();
                    }
                }

                break;

            case GTK_RESPONSE_CANCEL:
            case GTK_RESPONSE_DELETE_EVENT:
                cont = FALSE;

                break;

            default:
                g_warning("Unknown respons from dialog!");

                break;
        }
    }

    gtk_widget_hide(GTK_WIDGET(dialog_new));
}

int
main(int argc, char *argv[])
{
    GError *err = NULL;
    GtkWidget *window_main;

    gswe_init(EPHEDIR);
    gtk_init(&argc, &argv);

    builder = gtk_builder_new();

    if (gtk_builder_add_from_file(builder, UI_FILE, &err) == 0) {
        g_error("Builder error: %s", err->message);
    }

    window_main = GTK_WIDGET(gtk_builder_get_object(builder, "window_main"));
    gtk_window_set_title(GTK_WINDOW(window_main), "Astrognome");
    gtk_builder_connect_signals(builder, NULL);

    gtk_widget_show(window_main);
    gtk_main();

    g_object_unref(G_OBJECT(builder));

    return 0;
}

