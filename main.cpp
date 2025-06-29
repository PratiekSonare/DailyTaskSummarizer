#include <gtkmm/application.h>
#include <gtkmm/window.h>
#include <gtkmm/cssprovider.h>
#include <gtkmm/stylecontext.h>
#include <gdkmm/screen.h>
#include "MainWindow.h"  // or your main window class

int main(int argc, char *argv[]) {
    auto app = Gtk::Application::create(argc, argv, "org.noteontop");

    // Load and apply the CSS
    auto css_provider = Gtk::CssProvider::create();
    css_provider->load_from_path("/home/pratiek/Downloads/Bakchod_Projects/bakchod-notetaking/style.css");

    Gtk::StyleContext::add_provider_for_screen(
        Gdk::Screen::get_default(),
        css_provider,
        GTK_STYLE_PROVIDER_PRIORITY_USER
    );

    MainWindow window;
    return app->run(window);
}
