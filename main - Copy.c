#include <stdlib.h>
#include <windows.h>
#include <cairo.h>
#include <gtk/gtk.h>
#include <math.h>
#include <string.h>

typedef struct vector
{
    int x;
    int y;
} Vector;

typedef struct fish
{
    Vector position;
    Vector velocity;
    cairo_surface_t *image;
    int color;

    struct fish *next;
} Fish;

typedef struct CairoWindow
{
    GtkWidget *window;
    guint fishCount;
    Fish *school;
} CairoWindow;

typedef struct window
{
    GtkWidget *window;
    Fish *boids;
    guint sourceid;
    guint speed;
    guint refreshRate;
    guint actualColor;
    GtkWidget *comboBox;
    cairo_surface_t *backgroundImage;
} Window;

Window *Interface;

void setVector(Vector *v, int a, int b)
{
    v->x = a;
    v->y = b;
}

Vector addVector(Vector a, Vector b)
{

    a.x += b.x;
    a.y += b.y;
    return a;
}

Vector subVector(Vector a, Vector b)
{
    a.x -= b.x;
    a.y -= b.y;

    return a;
}

Vector divVector(Vector a, int b)
{

    if(b)
    {
        a.x /= b;
        a.y /= b;
    }

    return a;
}

int distance(Vector a,Vector b)
{
    a.x-=b.x;
    a.y-=b.y;

    return sqrt(pow(a.x,2) + pow(a.y,2));
}

void createFish(int x, int y)
{
    puts("Create");
    Fish *newFish = (Fish *)malloc(sizeof(Fish));
    setVector(&(newFish->position), x, y);
    newFish->color = Interface->actualColor;
    newFish->next = Interface->boids;

    Interface->boids = newFish;
    puts("Finish");
}

///	Moving a fish toward the center of the flock
Vector rule1(Fish *fish)
{
    Vector centerOfMass;

    setVector(&centerOfMass,0,0);

    int fishCount = 1;

    puts("For");

    for(Fish *boid = Interface->boids; boid != NULL; boid = boid->next)
    {
        if(boid == fish)
            continue;

        centerOfMass = addVector(centerOfMass,boid->position);
        fishCount++;
    }
    puts("EndFor");

    puts("divVector");
    centerOfMass = divVector(centerOfMass, fishCount);

    puts("subVector");
    centerOfMass = subVector(centerOfMass,fish->position);

    puts("divVector");
    centerOfMass = divVector(centerOfMass,1000);
    puts("End");

    return centerOfMass;
}

///	Avoid colliding into other fishs
Vector rule2(Fish *fish)
{
    Vector c;
    setVector(&c,0,0);

    for(Fish *boid = Interface->boids; boid != NULL; boid = boid->next)
    {
        if(fish == boid)
            continue;

        if( distance(boid->position,fish->position) < 30)
        {
            c = subVector(c,subVector(boid->position,fish->position));
        }
    }

    return c;
}

Vector rule3(Fish *fish)
{
    Vector perceivedVelocity;
    int fishCount = 0;

    for(Fish *boid = Interface->boids; boid != NULL; boid = boid->next)
    {
        if(fish == boid)
            continue;

        perceivedVelocity = addVector(perceivedVelocity,boid->velocity);
        fishCount++;
    }

    perceivedVelocity = divVector(perceivedVelocity,fishCount);

    return divVector(subVector(perceivedVelocity,fish->velocity),8);
}

char *concat(const char *s1, const char *s2)
{
    char *result = malloc(strlen(s1) + strlen(s2) + 1); // +1 for the null-terminator
    // in real code you would check for errors in malloc here
    strcpy(result, s1);
    strcat(result, s2);
    return result;
}

void drawFish(Fish *fish, cairo_t *cr)
{
    puts("Definir Image");
    definir_image(fish);

    puts("Cairo Image");
    cairo_set_source_surface(cr, fish->image, fish->position.x, fish->position.y);

    cairo_paint(cr);
}

void checkLimits(Vector *position)
{
    int xMAX,yMAX;
    gtk_window_get_size(GTK_WINDOW(Interface->window),&xMAX,&yMAX);

    if( (position->y < 0) && (position->x > 0) && (position->x < xMAX))
    {
        position->y = yMAX;
    }
    else if( (position->y > yMAX) && (position->x > 0) && (position->x < xMAX))
    {
        position->y = 0;
    }
    else if( position->x < 0 && 0<position->y && position->y<yMAX)
    {
        position->x = xMAX;
    }
    else if( position->x > xMAX && 0<position->y && position->y<yMAX)
    {
        position->x = 0;
    }
    else if( position->y < 0 && position->x > xMAX)
    {
        position->y = yMAX;
        position->x = 0;
    }
    else if( position->y > yMAX && position->x < 0)
    {
        position->y = 0;
        position->x = xMAX;
    }
    else if( position->x < 0 && position->y< 0)
    {
        position->x = xMAX;
        position->y = yMAX;
    }
    else if( position->x > xMAX && position->y > yMAX)
    {
        position->x = 0;
        position->y = 0;
    }

}

void moveAllBoidsToNewPositions(cairo_t *cr)
{
    Vector v1, v2, v3;

    for(Fish *fish = Interface->boids; fish != NULL; fish = fish->next)
    {
        puts("rules");
        puts("1");
        v1 = rule1(fish);
        puts("2");
        v2 = rule2(fish);
        puts("3");
        v3 = rule3(fish);

        puts("calcs");
        fish->velocity = addVector(fish->velocity, v1);
        fish->velocity = addVector(fish->velocity, v2);
        fish->velocity = addVector(fish->velocity, v3);

        puts("postion");
        fish->position = addVector(fish->position, fish->velocity);

        puts("Limits");
        checkLimits(&(fish->position));

        puts("Drawing");
        drawFish(fish,cr);
        puts("Done !");
    }
}

gboolean dessine(GtkWidget *widget, cairo_t *cr)
{
    srand(time(NULL));

    cairo_set_source_surface(cr, Interface->backgroundImage, 0, 0);
    cairo_paint(cr);

    moveAllBoidsToNewPositions(cr);

    return FALSE;
}

void radio_button_selected (GtkWidget *widget, gpointer data)
{
    if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (widget)))
    {
        GSList *group = gtk_radio_button_get_group (GTK_RADIO_BUTTON (widget));
        Interface->actualColor= g_slist_index (group, widget);
    }
}

void on_changed (GtkComboBox *combo_box,gpointer data)
{
   // Interface->selected = atoi(gtk_combo_box_text_get_active_text (GTK_COMBO_BOX_TEXT(combo_box)));
}

void definir_image(Fish *fish)
{
    char *src;
    char *color;
     puts("xy");

    int x = fish->position.x;
    int y = fish->position.y;

    puts("calc Angle");
    if(!x)x=1;

    double angle =  (180*atan(y/x))/3.141592654;

    printf("\n%lf %d %d",angle,y,x);

    int choice;

    if(angle >= 67.5 && angle < 112.5 )
        choice = 0;
    else if(angle >= 112.5 && angle < 157.5 )
        choice = 7;
    else if(angle >= 157.5 && angle < 202.5 )
        choice = 6;
    else if(angle >= 202.5 && angle < 247.5  )
        choice = 5;
    else if(angle >= 247.5 && angle < 292.5 )
        choice = 4;
    else if(angle >= 292.5 && angle < 337.5 )
        choice = 3;
    else if((angle >= 337.5 && angle <= 360)||( angle >= 0 && angle < 22.5 ))
        choice = 2;
    else if(angle >= 22.5 && angle < 67.5 )
        choice = 1;

    switch(choice)
    {
    case 6:
        src="left.png";
        break;
    case 7:
        src="up_left.png";
        break;
    case 0:
        src="up.png";
        break;
    case 1:
        src="up_right.png";
        break;
    case 2:
        src="right.png";
        break;
    case 3:
        src="down_right.png";
        break;
    case 4:
        src="down.png";
        break;
    case 5:
        src="down_left.png";
        break;
    }
     puts("switch2");

    switch(fish->color)
    {
    case 0:
        color="blue\\";
        break;
    case 1:
        color="orange\\";
        break;
    default:
        color="black\\";

    }

    puts("test");
    src=concat(color,src);
    puts("test");

    printf("\n is %s",src);
    fish->image = cairo_image_surface_create_from_png(src);
}

gboolean appele_successive_de_dessin(GtkWidget *widget,gpointer data)
{
    gtk_widget_queue_draw(widget);
    return TRUE;
}

void Arrete(GtkWidget *button,gpointer data)
{
    g_source_remove(Interface->sourceid);
}

void lance(GtkWidget *button,gpointer data)
{
    g_source_remove(Interface->sourceid);
    Interface->sourceid = g_timeout_add(Interface->refreshRate, (GSourceFunc) appele_successive_de_dessin, (gpointer) Interface->window);
}

void change_vitesse(GtkRange *spin,gpointer data)
{
    Interface->refreshRate = gtk_spin_button_get_value (spin);
    g_source_remove(Interface->sourceid);
    Interface->sourceid = g_timeout_add(Interface->refreshRate, (GSourceFunc) appele_successive_de_dessin, (gpointer) Interface->window);
}

void change_deplacement(GtkRange *spin,gpointer data)
{
    Interface->speed = gtk_spin_button_get_value (spin);
    g_source_remove(Interface->sourceid);
    Interface->sourceid = g_timeout_add(Interface->refreshRate, (GSourceFunc) appele_successive_de_dessin, (gpointer) Interface->window);
}

gboolean clicked(GtkWidget *widget, GdkEventButton *event,gpointer data)
{
    if (event->button == 1)///lift-click
    {
///        int x = rand() % xMAX;
///        int y = rand() % yMAX;
        createFish(event->x,event->y);
    }

    if (event->button == 3) ///right-click
    {

    }
    return TRUE;
}

int main(int argc, char *argv[])
{
    Interface = (Window *)malloc(sizeof(Window));

    GtkWidget *buttonStart;
    GtkWidget *buttonStop;
    GtkWidget *buttonAdd;

    GtkWidget *speedSpin;
    GtkWidget *refreshSpin;

    GtkWidget *darea;

    gtk_init(&argc, &argv);


    Interface->window = gtk_window_new(GTK_WINDOW_TOPLEVEL);

    int dwWidth = GetSystemMetrics(SM_CXSCREEN);
    int dwHeight = GetSystemMetrics(SM_CYSCREEN);

    gtk_widget_set_size_request((Interface->window),dwWidth,dwHeight);
    gtk_window_set_resizable(GTK_WINDOW(Interface->window),0);
    gtk_window_set_position(GTK_WINDOW(Interface->window), GTK_WIN_POS_CENTER);
    gtk_window_set_default_size(GTK_WINDOW(Interface->window), 400, 300);
    gtk_window_set_title(GTK_WINDOW(Interface->window), "Fish Schooling");

    GtkAdjustment *adjustment = gtk_adjustment_new (60.0, 0.0, 1000.0, 1.0, 5.0, 0.0);
    GtkAdjustment *adjustment2 = gtk_adjustment_new (10.0, 0.0, 100.0, 1.0, 5.0, 0.0);

    buttonStart = gtk_button_new_with_label("Start");
    buttonStop = gtk_button_new_with_label("Stop");
    buttonAdd = gtk_button_new_with_label("Add");

    refreshSpin = gtk_spin_button_new (adjustment, 1.0, 0);
    speedSpin =    gtk_spin_button_new (adjustment2, 1.0, 0);

    GtkWidget *  radioButtonBox = gtk_box_new (GTK_ORIENTATION_VERTICAL, 2);
    GtkWidget*      combo_box = gtk_combo_box_text_new();


    GtkWidget * radio1 = gtk_radio_button_new_with_label(NULL,"black");
    GtkWidget *radio2 = gtk_radio_button_new_with_label_from_widget (GTK_RADIO_BUTTON (radio1),"orange");
    GtkWidget *radio3 = gtk_radio_button_new_with_label_from_widget (GTK_RADIO_BUTTON (radio2),"blue");

    gtk_combo_box_set_title(GTK_COMBO_BOX(combo_box),"select fish");

    gtk_box_set_homogeneous (GTK_BOX (radioButtonBox), TRUE);

    gtk_combo_box_set_active (GTK_COMBO_BOX (combo_box), 0);

    darea = gtk_drawing_area_new();

    gtk_widget_add_events(Interface->window, GDK_BUTTON_PRESS_MASK);

    ///Init Cairo Window
    Interface->boids = NULL;
    Interface->refreshRate = 60;
    Interface->speed = 1;
    Interface->actualColor  =2;

    Interface->backgroundImage = cairo_image_surface_create_from_png("blue.png");

    /// MENU
    GtkWidget *pMenuBar;
    GtkWidget *pMenu;
    GtkWidget *pMenuItem;


    pMenuBar = gtk_menu_bar_new();
    /** Premier sous-menu **/
    pMenu = gtk_menu_new();

    pMenuItem = gtk_menu_item_new_with_label("Nouveau");
    gtk_menu_shell_append(GTK_MENU_SHELL(pMenu), pMenuItem);

    pMenuItem = gtk_menu_item_new_with_label("Ouvrir");
    gtk_menu_shell_append(GTK_MENU_SHELL(pMenu), pMenuItem);

    pMenuItem = gtk_menu_item_new_with_label("Enregistrer");
    gtk_menu_shell_append(GTK_MENU_SHELL(pMenu), pMenuItem);

    pMenuItem = gtk_menu_item_new_with_label("Fermer");
    gtk_menu_shell_append(GTK_MENU_SHELL(pMenu), pMenuItem);

    pMenuItem = gtk_menu_item_new_with_label("Quitter");

//    g_signal_connect(G_OBJECT(pMenuItem), "activate", G_CALLBACK(OnQuitter),(GtkWidget*) pWindow);

    gtk_menu_shell_append(GTK_MENU_SHELL(pMenu), pMenuItem);

    pMenuItem = gtk_menu_item_new_with_label("Fichier");

    gtk_menu_item_set_submenu(GTK_MENU_ITEM(pMenuItem), pMenu);

    gtk_menu_shell_append(GTK_MENU_SHELL(pMenuBar), pMenuItem);

    /** Second sous-menu **/

    pMenu = gtk_menu_new();

    pMenuItem = gtk_menu_item_new_with_label("A propos de...");

    gtk_menu_shell_append(GTK_MENU_SHELL(pMenu), pMenuItem);

    pMenuItem = gtk_menu_item_new_with_label("?");

    gtk_menu_item_set_submenu(GTK_MENU_ITEM(pMenuItem), pMenu);

    gtk_menu_shell_append(GTK_MENU_SHELL(pMenuBar), pMenuItem);

    // SIGNALS
    g_signal_connect(Interface->window, "destroy",
                     G_CALLBACK(gtk_main_quit),
                     NULL);

    g_signal_connect(G_OBJECT(darea), "draw",
                     G_CALLBACK(dessine),
                     NULL);


    g_signal_connect (combo_box,
                      "changed",
                      G_CALLBACK (on_changed),
                      NULL);


    g_signal_connect(Interface->window, "button-press-event",
                     G_CALLBACK(clicked),
                     NULL);

    g_signal_connect (GTK_BUTTON (buttonStart),
                      "clicked",
                      G_CALLBACK (lance),
                      NULL);

    g_signal_connect (GTK_BUTTON (buttonStop),
                      "clicked",
                      G_CALLBACK (Arrete),
                      NULL);

 /*   g_signal_connect (GTK_BUTTON (buttonAdd),
                      "clicked",
                      G_CALLBACK (ajouter_poisson_en_emplacement_aleatoire),
                      NULL);
*/
    g_signal_connect (GTK_BUTTON (radio1),
                      "clicked",
                      G_CALLBACK (radio_button_selected),
                      NULL);

    g_signal_connect (GTK_BUTTON (radio2),
                      "clicked",
                      G_CALLBACK (radio_button_selected),
                      NULL);

    g_signal_connect (GTK_BUTTON (radio3),
                      "clicked",
                      G_CALLBACK (radio_button_selected),
                      NULL);

/*    g_signal_connect(G_OBJECT(refreshSpin), "value-changed",G_CALLBACK (change_vitesse),NULL);

    g_signal_connect(G_OBJECT(speedSpin), "value-changed",G_CALLBACK (change_deplacement),NULL);
*/
    GtkWidget *fenetreBox=gtk_box_new(GTK_ORIENTATION_VERTICAL,0);
    GtkWidget *optionBox=gtk_box_new(GTK_ORIENTATION_HORIZONTAL,0);
    GtkWidget *horBox=gtk_box_new(GTK_ORIENTATION_HORIZONTAL,0);
    GtkWidget *reglesBox=gtk_box_new(GTK_ORIENTATION_VERTICAL,0);

    gtk_box_pack_start(GTK_BOX(reglesBox),buttonStart,FALSE,FALSE,0);
    gtk_box_pack_start(GTK_BOX(reglesBox),buttonStop,FALSE,FALSE,0);
    gtk_box_pack_start(GTK_BOX(reglesBox),refreshSpin,FALSE,FALSE,0);
    gtk_box_pack_start(GTK_BOX(reglesBox),speedSpin,FALSE,FALSE,0);
    gtk_box_pack_start (GTK_BOX (radioButtonBox), radio1,FALSE,FALSE,0);
    gtk_box_pack_start (GTK_BOX (radioButtonBox), radio2,FALSE,FALSE,0);
    gtk_box_pack_start (GTK_BOX (radioButtonBox), radio3,FALSE,FALSE,0);
    gtk_box_pack_start (GTK_BOX (reglesBox), radioButtonBox,FALSE,FALSE,0);
    gtk_box_pack_start(GTK_BOX(reglesBox),buttonAdd,FALSE,FALSE,0);
    gtk_box_pack_start (GTK_BOX (reglesBox), combo_box,FALSE,FALSE,0);

    gtk_box_pack_start(GTK_BOX(fenetreBox),pMenuBar,FALSE,FALSE,0);
    gtk_box_pack_start(GTK_BOX(fenetreBox),horBox,TRUE,TRUE,0);
    gtk_box_pack_start(GTK_BOX(horBox),darea,TRUE,TRUE,0);
    gtk_box_pack_start(GTK_BOX(horBox),reglesBox,FALSE,FALSE,0);

    gtk_container_add(GTK_CONTAINER(Interface->window), fenetreBox);
    gtk_widget_show_all(Interface->window);

    gtk_main();

    return 0;
}
