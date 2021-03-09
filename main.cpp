/*****************************************************************************\
 * TP CPE, 4ETI, TP synthese d'images
 * --------------
 * OLLIER
 * LAKKIS
 * FAUT
 * Programme principal des appels OpenGL
 \*****************************************************************************/

#include <iostream>
#include <cstdlib>
#include <cstdio>
#include <cmath>
#include <string> // pour utiliser des strings
using namespace std; 
#define GLEW_STATIC 1
#include <GL/glew.h>

#if defined(__APPLE__)
#include <OpenGL/gl3.h>
#define __gl_h_
#include <GLUT/glut.h>
#define MACOSX_COMPATIBILITY GLUT_3_2_CORE_PROFILE
#else
#include <GL/glut.h>
#define MACOSX_COMPATIBILITY 0
#endif

#include "glhelper.hpp"
#include "mat4.hpp"
#include "vec3.hpp"
#include "vec2.hpp"
#include "triangle_index.hpp"
#include "vertex_opengl.hpp"
#include "image.hpp"
#include "mesh.hpp"

 /*****************************************************************************\
  * Variables globales
  *
  *
  \*****************************************************************************/

  //identifiant du shader
GLuint shader_program_id;
GLuint gui_program_id;

int vie = 2; //nombre de vie de base


//Matrice de transformation
struct transformation
{
    mat4 rotation;
    vec3 rotation_center;
    vec3 translation;

    transformation() :rotation(), rotation_center(), translation() {}
};

//Informations correspondants à un model
struct model
{
    GLuint vao = 0;
    GLuint texture_id = 0;
    int nbr_triangle;
    transformation transformation_model;
    vec3 angle = vec3(0.0f, 0.0f, 0.0f); // angle suivant x, y et z
    // rotation autour de y non implémentée
};

//Liste des modèls
model model_dinosaure;
model model_sol;
model model_missile;
model model_arbre;
model model_gagne;
model model_perdu;

transformation tab_transfo_missile[5]; // création d'un tab contenant les missiles
int missile_count = 0;

//Transformation de la vue (camera)
transformation transformation_view;
float angle_view = 0.0f;

//Matrice de projection
mat4 projection;



void load_texture(const char* filename, GLuint* texture_id);

void init_model_1();
void init_model_2();
void init_model_3();
void init_model_4();
void init_model_5();
void init_model_6();




void draw_model(model model_id);
void draw_model_3();
void draw_model_4();
void draw_model_5();
void draw_model_6();





// Text drawing Structure
struct text {
    GLuint vao = 0;              // Vertex array id
    GLuint texture_id = 0;       // Well, texture id...
    std::string value;           // Value of the text to display
    vec2 bottomLeft;
    vec2 topRight;
};
text text_to_draw;

void init_text(text* t) {
    vec3 p0 = vec3(0.0f, 0.0f, 0.0f);
    vec3 p1 = vec3(0.0f, 1.0f, 0.0f);
    vec3 p2 = vec3(1.0f, 1.0f, 0.0f);
    vec3 p3 = vec3(1.0f, 0.0f, 0.0f);

    vec3 geometrie[4] = { p0, p1, p2, p3 };
    triangle_index index[2] = { triangle_index(0, 1, 2), triangle_index(0, 2, 3) };

    glGenVertexArrays(1, &(t->vao));                                              CHECK_GL_ERROR();
    glBindVertexArray(t->vao);                                                  CHECK_GL_ERROR();

    GLuint vbo;
    glGenBuffers(1, &vbo);                                                       CHECK_GL_ERROR();
    glBindBuffer(GL_ARRAY_BUFFER, vbo);                                          CHECK_GL_ERROR();
    glBufferData(GL_ARRAY_BUFFER, sizeof(geometrie), geometrie, GL_STATIC_DRAW);   CHECK_GL_ERROR();

    glEnableVertexAttribArray(0); CHECK_GL_ERROR();
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0); CHECK_GL_ERROR();

    GLuint vboi;
    glGenBuffers(1, &vboi);                                                      CHECK_GL_ERROR();
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vboi);                                 CHECK_GL_ERROR();
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(index), index, GL_STATIC_DRAW);   CHECK_GL_ERROR();

    load_texture("data/fontB.tga", &t->texture_id);
}

void draw_texts(text* text_t) {
    vec2 size = (text_t->topRight - text_t->bottomLeft) / float(text_t->value.size());
    //  std::cout << size << " " << text_t->bottomLeft << std::endl;

    //Send uniforma parameters

    GLint loc_size = glGetUniformLocation(gui_program_id, "size"); CHECK_GL_ERROR();
    if (loc_size == -1) std::cerr << "Pas de variable uniforme : size" << std::endl;
    glUniform2f(loc_size, size.x, size.y);    CHECK_GL_ERROR();

    glBindVertexArray(text_t->vao); CHECK_GL_ERROR();

    for (int i = 0; i < text_t->value.size(); ++i)
    {
        GLint loc_start = glGetUniformLocation(gui_program_id, "start"); CHECK_GL_ERROR();
        if (loc_start == -1) std::cerr << "Pas de variable uniforme : start" << std::endl;
        glUniform2f(loc_start, text_t->bottomLeft.x + i * size.x, text_t->bottomLeft.y);    CHECK_GL_ERROR();

        GLint loc_char = glGetUniformLocation(gui_program_id, "c"); CHECK_GL_ERROR();
        if (loc_char == -1) std::cerr << "Pas de variable uniforme : c" << std::endl;
        glUniform1i(loc_char, (int)text_t->value[i]);    CHECK_GL_ERROR();
        glBindTexture(GL_TEXTURE_2D, text_t->texture_id);                       CHECK_GL_ERROR();
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);                    CHECK_GL_ERROR();
    }
}


static void init()
{
 // Chargement du shader
    gui_program_id = glhelper::create_program_from_file(
        "shaders/simple.vert",
        "shaders/simple.frag"); CHECK_GL_ERROR();

   
    shader_program_id = glhelper::create_program_from_file(
        "shaders/shader.vert",
        "shaders/shader.frag"); CHECK_GL_ERROR();
    glUseProgram(shader_program_id);

    //matrice de projection
    projection = matrice_projection(60.0f * M_PI / 180.0f, 1.0f, 0.01f, 100.0f);
    GLint loc_projection = glGetUniformLocation(shader_program_id, "projection"); CHECK_GL_ERROR();
    if (loc_projection == -1) std::cerr << "Pas de variable uniforme : projection" << std::endl;
    glUniformMatrix4fv(loc_projection, 1, false, pointeur(projection)); CHECK_GL_ERROR();

    //centre de rotation de la 'camera' (les objets sont centres en z=-2)
    transformation_view.rotation_center =  vec3(0.0f, 0.0f, -2.0f);
     
       

    //activation de la gestion de la profondeur
    glEnable(GL_DEPTH_TEST); CHECK_GL_ERROR();

    // Charge modele 1 sur la carte graphique
    init_model_1();

    // Charge modele 2 sur la carte graphique
    init_model_2();

    // Charge modele 3 sur la carte graphique (missile)
    init_model_3();

    // Charge modele 4 sur la carte graphique (arbre)
    init_model_4();

    // Charge modele 5 sur la carte graphique (gagné)
    init_model_5();

    // Charge modele 6 sur la carte graphique (perdu)
    init_model_6();

    tab_transfo_missile [missile_count]=model_missile.transformation_model;
    tab_transfo_missile[missile_count].translation.z += -3.;
    tab_transfo_missile[missile_count].translation.x += -2.;
    missile_count ++;

    tab_transfo_missile[missile_count] = model_missile.transformation_model;
    tab_transfo_missile[missile_count].translation.z += -2.;
    missile_count++;

    tab_transfo_missile[missile_count].translation.z += -1.;
    tab_transfo_missile[missile_count].translation.x += -1.;
    missile_count++;

    tab_transfo_missile[missile_count].translation.z += -1.;
    tab_transfo_missile[missile_count].translation.x += -2.5;
    missile_count++;

    tab_transfo_missile[missile_count].translation.z += -1.5;
    tab_transfo_missile[missile_count].translation.x += 1.5;
    missile_count++;





    transformation_view.translation.z = -25.5; // on recule la caméra pour avoir une meilleure vue sur la map 
    model_dinosaure.transformation_model.translation.z = +22;  // on recule le dinosaure pour les memes raisons
    model_dinosaure.angle.y = 3.15; // on chosit de  mettre le dinosaure dos à nous, on change donc son angle pour le tourner
    model_dinosaure.transformation_model.rotation = matrice_rotation(model_dinosaure.angle.y, 0.0f, 1.0f, 0.0f); // on effectue le changement de rotation avec cette ligne sinon le dinosaure apparait sans aucun changement.
    
 

     


    // placement du text violet
    text_to_draw.bottomLeft = vec2(-0.7, -0.7);
    text_to_draw.topRight = vec2(0.7, 0.35);
 



    init_text(&text_to_draw);
}

//Fonction d'affichage
static void display_callback()
{
    //effacement des couleurs du fond d'ecran
    glClearColor(0.5f, 0.6f, 0.9f, 1.0f); CHECK_GL_ERROR();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); CHECK_GL_ERROR();
    glEnable(GL_DEPTH_TEST);


    //définie le programme à utiliser
    glUseProgram(shader_program_id);
   


    // Affecte les parametres uniformes de la vue (identique pour tous les modeles de la scene)
    {

        //envoie de la rotation
        GLint loc_rotation_view = glGetUniformLocation(shader_program_id, "rotation_view"); CHECK_GL_ERROR();
        if (loc_rotation_view == -1) std::cerr << "Pas de variable uniforme : rotation_view" << std::endl;
        glUniformMatrix4fv(loc_rotation_view, 1, false, pointeur(transformation_view.rotation)); CHECK_GL_ERROR();

        //envoie du centre de rotation
        vec3 cv = transformation_view.rotation_center;
        GLint loc_rotation_center_view = glGetUniformLocation(shader_program_id, "rotation_center_view"); CHECK_GL_ERROR();
        if (loc_rotation_center_view == -1) std::cerr << "Pas de variable uniforme : rotation_center_view" << std::endl;
        glUniform4f(loc_rotation_center_view, cv.x, cv.y, cv.z, 0.0f); CHECK_GL_ERROR();

        //envoie de la translation
        vec3 tv = transformation_view.translation;
        GLint loc_translation_view = glGetUniformLocation(shader_program_id, "translation_view"); CHECK_GL_ERROR();
        if (loc_translation_view == -1) std::cerr << "Pas de variable uniforme : translation_view" << std::endl;
        glUniform4f(loc_translation_view, tv.x, tv.y, tv.z, 0.0f); CHECK_GL_ERROR();
    }

    // Affiche le modele dinosaure
    draw_model(model_dinosaure);
    // Affiche le modele sol
    draw_model(model_sol);
    // Affiche le modele numero 2 (monstre)
    
    
    
    for (int i = 0; i < missile_count; i++) {

        model_missile.transformation_model = tab_transfo_missile[i];
        draw_model(model_missile);

    }
    
    int i;
     model_arbre.transformation_model.translation.z = -14.00; // placement du premier arbre

     for (i = 0; i < 51; i++) { // boucle pour créer les 2 rangées d abres
    
        model_arbre.transformation_model.translation.x = -3.0;
        draw_model(model_arbre);
        model_arbre.transformation_model.translation.x = -4.0;
        draw_model(model_arbre);

        model_arbre.transformation_model.translation.x = +3.0;
        draw_model(model_arbre);
        model_arbre.transformation_model.translation.x = +4.0;
        draw_model(model_arbre);

       
        model_arbre.transformation_model.translation.z += 0.8; // espace entre 2 arbres
     
    } 


     draw_model(model_gagne);
     model_gagne.transformation_model.translation.x = +20.0;

     draw_model(model_perdu);
     model_perdu.transformation_model.translation.x = +30.0;

    glDisable(GL_DEPTH_TEST);
    glUseProgram(gui_program_id);
   draw_texts(&text_to_draw);

    //Changement de buffer d'affichage pour eviter un effet de scintillement
    glutSwapBuffers();
}

/*****************************************************************************\
 * keyboard_callback                                                         *
 \*****************************************************************************/
static void keyboard_callback(unsigned char key, int, int)
{
    float d_angle = 0.1f;
    float dz = 0.5f;

    //quitte le programme si on appuie sur les touches 'q', 'Q', ou 'echap'
    //enregistre l'image si on appuie sur la touche 'p'
    switch (key)
    {
    case 'p':
        glhelper::print_screen();
        break;
    case 'q':
    case 'Q':
    case 27:
        exit(0);
        break;

    case 'o':
        model_dinosaure.angle.x += d_angle;
        break;
    case 'l':
        model_dinosaure.angle.x -= d_angle;
        break;

    case 'k':
        model_dinosaure.angle.y += d_angle;
        break;
    case 'm':
        model_dinosaure.angle.y -= d_angle;
        break;


    case 's':
        angle_view += d_angle;
        break;
    case 'f':
        angle_view -= d_angle;
        break;


    case 'e':
        transformation_view.translation.z += dz;
        break;
    case 'd':
        transformation_view.translation.z -= dz;
        break;
    }

    model_dinosaure.transformation_model.rotation = matrice_rotation(model_dinosaure.angle.y, 0.0f, 1.0f, 0.0f) * matrice_rotation(model_dinosaure.angle.x, 1.0f, 0.0f, 0.0f);
    transformation_view.rotation = matrice_rotation(angle_view, 0.0f, 1.0f, 0.0f);
    // Des exemples de camera vous sont données dans le programme 10

}

/*****************************************************************************\
 * special_callback                                                          *
 \*****************************************************************************/
static void special_callback(int key, int, int)
{
    float dL = 0.03f;
    switch (key)
    { // pas besoin de faire sortir le dino en dehors de la map 
  /**  case GLUT_KEY_UP:
        model_dinosaure.transformation_model.translation.y += dL; //rotation avec la touche du haut
        break;
    case GLUT_KEY_DOWN:
        model_dinosaure.transformation_model.translation.y -= dL; //rotation avec la touche du bas
        break; **/
    case GLUT_KEY_LEFT:
        model_dinosaure.transformation_model.translation.x -= dL; //rotation avec la touche de gauche
        break;
    case GLUT_KEY_RIGHT:
        model_dinosaure.transformation_model.translation.x += dL; //rotation avec la touche de droite
        break;
    }
}

int a; // pour les boucles f
int b;
int c;
int d;
/*****************************************************************************\
 * timer_callback                                                            *
 \*****************************************************************************/
static void timer_callback(int)
{
    //demande de rappel de cette fonction dans 25ms
    glutTimerFunc(25, timer_callback, 0);

    //reactualisation de l'affichage
    glutPostRedisplay();

    //std::cout <<tab_transfo_missile[0].translation.x << std::endl;



    // on gere ici la vitesse des missiles
  tab_transfo_missile[0].translation.z += 0.07;
  tab_transfo_missile[1].translation.z += 0.05;
  tab_transfo_missile[2].translation.z += 0.04;
  tab_transfo_missile[3].translation.z += 0.03;
  tab_transfo_missile[4].translation.z += 0.09;

 

  
      
      for (int a = 0; a < missile_count + 1; a++) // collision
      {


          if (abs(model_dinosaure.transformation_model.translation.x - tab_transfo_missile[a].translation.x) < 0.35 && abs(model_dinosaure.transformation_model.translation.z - tab_transfo_missile[a].translation.z) < 0.35)
          {
              tab_transfo_missile[a].translation.x += 30.;
              vie -= 1;
            
              std::cout << vie << std::endl;
             
          }


      }

      string test = to_string(vie);  // permet de mettre la valeur "vie" en une chaine de caractère 

      text_to_draw.value = " vous avez " + test + " vie(s)";
      
      if (vie > 0 && tab_transfo_missile[3].translation.z > 23 ) { // on attend le dernier missile et on vérifie la vie

              model_gagne.transformation_model.translation.x = -5.0; 
              model_gagne.transformation_model.translation.y = 5.0;

              for (int d = 0; d < missile_count + 1; d++) // disparition des missiles 
              {
                  tab_transfo_missile[d].translation.z = 50;

              }


          }

     

      if (vie <= 0) {

          model_perdu.transformation_model.translation.x = -6.5;
          model_perdu.transformation_model.translation.y = 5.0;

          for (int c = 0; c < missile_count + 1; c++) 
          {
              tab_transfo_missile[c].translation.z = 50;

          }


      }


  // on empeche ici de faire sortir le dinosaure de la map sur les cotés 
  if (model_dinosaure.transformation_model.translation.x > 2.7)
      model_dinosaure.transformation_model.translation.x = 2.7;

  if (model_dinosaure.transformation_model.translation.x < -2.7)
      model_dinosaure.transformation_model.translation.x = -2.7;



}

int main(int argc, char** argv)
{
    //**********************************************//
    //Lancement des fonctions principales de GLUT
    //**********************************************//

    //initialisation
    glutInit(&argc, argv);

    //Mode d'affichage (couleur, gestion de profondeur, ...)
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH | MACOSX_COMPATIBILITY);

    //Taille de la fenetre a l'ouverture
    glutInitWindowSize(600, 600);

    //Titre de la fenetre
    glutCreateWindow("OpenGL");

    //Fonction de la boucle d'affichage
    glutDisplayFunc(display_callback);

    //Fonction de gestion du clavier
    glutKeyboardFunc(keyboard_callback);
    


    //Fonction des touches speciales du clavier (fleches directionnelles)
    glutSpecialFunc(special_callback);

    //Fonction d'appel d'affichage en chaine
    glutTimerFunc(25, timer_callback, 0);

    //Option de compatibilité
    glewExperimental = true;

    //Initialisation des fonctions OpenGL
    glewInit();

    //Affiche la version openGL utilisée
    std::cout << "OpenGL: " << (GLchar*)(glGetString(GL_VERSION)) << std::endl;

    //Notre fonction d'initialisation des donnees et chargement des shaders
    init();

    //Lancement de la boucle (infinie) d'affichage de la fenetre
    glutMainLoop();

    //Plus rien n'est execute apres cela
    return 0;
}

void draw_model(model m)
{

    //envoie des parametres uniformes
    {
        GLint loc_rotation_model = glGetUniformLocation(shader_program_id, "rotation_model"); CHECK_GL_ERROR();
        if (loc_rotation_model == -1) std::cerr << "Pas de variable uniforme : rotation_model" << std::endl;
        glUniformMatrix4fv(loc_rotation_model, 1, false, pointeur(m.transformation_model.rotation));    CHECK_GL_ERROR();

        vec3 c = m.transformation_model.rotation_center;
        GLint loc_rotation_center_model = glGetUniformLocation(shader_program_id, "rotation_center_model"); CHECK_GL_ERROR();
        if (loc_rotation_center_model == -1) std::cerr << "Pas de variable uniforme : rotation_center_model" << std::endl;
        glUniform4f(loc_rotation_center_model, c.x, c.y, c.z, 0.0f);                                 CHECK_GL_ERROR();

        vec3 t = m.transformation_model.translation;
        GLint loc_translation_model = glGetUniformLocation(shader_program_id, "translation_model"); CHECK_GL_ERROR();
        if (loc_translation_model == -1) std::cerr << "Pas de variable uniforme : translation_model" << std::endl;
        glUniform4f(loc_translation_model, t.x, t.y, t.z, 0.0f);                                     CHECK_GL_ERROR();
    }

    glBindVertexArray(m.vao); CHECK_GL_ERROR();

    //affichage
    {
        glBindTexture(GL_TEXTURE_2D, m.texture_id);                             CHECK_GL_ERROR();
        glDrawElements(GL_TRIANGLES, 3 * m.nbr_triangle, GL_UNSIGNED_INT, 0);     CHECK_GL_ERROR();
    }
}



void init_model_1()
{
    // Chargement d'un maillage a partir d'un fichier
    mesh m = load_obj_file("data/stegosaurus.obj");

    // Affecte une transformation sur les sommets du maillage
    float s = 0.2f;
    mat4 transform = mat4(s, 0.0f, 0.0f, 0.0f,
        0.0f, s, 0.0f, -0.9f,
        0.0f, 0.0f, s, -2.0f,
        0.0f, 0.0f, 0.0f, 1.0f);
    apply_deformation(&m, transform);

    // Centre la rotation du modele 1 autour de son centre de gravite approximatif
    model_dinosaure.transformation_model.rotation_center = vec3(0.0f, -0.5f, -2.0f);

    // Calcul automatique des normales du maillage
    update_normals(&m);
    // Les sommets sont affectes a une couleur blanche
    fill_color(&m, vec3(1.0f, 1.0f, 1.0f));

    //attribution d'une liste d'état (1 indique la création d'une seule liste)
    glGenVertexArrays(1, &model_dinosaure.vao);
    glBindVertexArray(model_dinosaure.vao);

    GLuint vbo;
    //attribution d'un buffer de donnees (1 indique la création d'un buffer)
    glGenBuffers(1, &vbo); CHECK_GL_ERROR();
    //affectation du buffer courant
    glBindBuffer(GL_ARRAY_BUFFER, vbo); CHECK_GL_ERROR();
    //copie des donnees des sommets sur la carte graphique
    glBufferData(GL_ARRAY_BUFFER, m.vertex.size() * sizeof(vertex_opengl), &m.vertex[0], GL_STATIC_DRAW); CHECK_GL_ERROR();

    // Active l'utilisation des données de positions (le 0 correspond à la location dans le vertex shader)
    glEnableVertexAttribArray(0); CHECK_GL_ERROR();
    // Indique comment le buffer courant (dernier vbo "bindé") est utilisé pour les positions des sommets
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(vertex_opengl), 0); CHECK_GL_ERROR();

    // Active l'utilisation des données de normales (le 1 correspond à la location dans le vertex shader)
    glEnableVertexAttribArray(1); CHECK_GL_ERROR();
    // Indique comment le buffer courant (dernier vbo "bindé") est utilisé pour les normales des sommets
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_TRUE, sizeof(vertex_opengl), (void*)sizeof(vec3)); CHECK_GL_ERROR();

    // Active l'utilisation des données de couleurs (le 2 correspond à la location dans le vertex shader)
    glEnableVertexAttribArray(2); CHECK_GL_ERROR();
    // Indique comment le buffer courant (dernier vbo "bindé") est utilisé pour les couleurs des sommets
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(vertex_opengl), (void*)(2 * sizeof(vec3))); CHECK_GL_ERROR();

    // Active l'utilisation des données de textures (le 3 correspond à la location dans le vertex shader)
    glEnableVertexAttribArray(3); CHECK_GL_ERROR();
    // Indique comment le buffer courant (dernier vbo "bindé") est utilisé pour les textures des sommets
    glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, sizeof(vertex_opengl), (void*)(3 * sizeof(vec3))); CHECK_GL_ERROR();

    GLuint vboi;
    //attribution d'un autre buffer de donnees
    glGenBuffers(1, &vboi); CHECK_GL_ERROR();
    //affectation du buffer courant (buffer d'indice)
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vboi); CHECK_GL_ERROR();
    //copie des indices sur la carte graphique
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, m.connectivity.size() * sizeof(triangle_index), &m.connectivity[0], GL_STATIC_DRAW); CHECK_GL_ERROR();

    // Nombre de triangles de l'objet 1
    model_dinosaure.nbr_triangle = m.connectivity.size();

    // Chargement de la texture
    load_texture("data/stegosaurus.tga", &model_dinosaure.texture_id);
}

void init_model_2()
{
    //Creation manuelle du model 2

    //coordonnees geometriques des sommets
    vec3 p0 = vec3(-25.0f, -0.9f, -25.0f);
    vec3 p1 = vec3(25.0f, -0.9f, -25.0f);
    vec3 p2 = vec3(25.0f, -0.9f, 25.0f);
    vec3 p3 = vec3(-25.0f, -0.9f, 25.0f);

    //normales pour chaque sommet
    vec3 n0 = vec3(0.0f, 1.0f, 0.0f);
    vec3 n1 = n0;
    vec3 n2 = n0;
    vec3 n3 = n0;

    //couleur pour chaque sommet
    vec3 c0 = vec3(1.0f, 1.0f, 1.0f);
    vec3 c1 = c0;
    vec3 c2 = c0;
    vec3 c3 = c0;

    //texture du sommet
    vec2 t0 = vec2(0.0f, 0.0f);
    vec2 t1 = vec2(1.0f, 0.0f);
    vec2 t2 = vec2(1.0f, 1.0f);
    vec2 t3 = vec2(0.0f, 1.0f);

    vertex_opengl v0 = vertex_opengl(p0, n0, c0, t0);
    vertex_opengl v1 = vertex_opengl(p1, n1, c1, t1);
    vertex_opengl v2 = vertex_opengl(p2, n2, c2, t2);
    vertex_opengl v3 = vertex_opengl(p3, n3, c3, t3);

    //tableau entrelacant coordonnees-normales
    vertex_opengl geometrie[] = { v0,v1,v2,v3 };

    //indice des triangles
    triangle_index tri0 = triangle_index(0, 1, 2);
    triangle_index tri1 = triangle_index(0, 2, 3);
    triangle_index index[] = { tri0,tri1 };
    model_sol.nbr_triangle = 2;

    //attribution d'une liste d'état (1 indique la création d'une seule liste)
    glGenVertexArrays(1, &model_sol.vao);
    glBindVertexArray(model_sol.vao);

    GLuint vbo;
    //attribution d'un buffer de donnees (1 indique la création d'un buffer)
    glGenBuffers(1, &vbo);                                             CHECK_GL_ERROR();
    //affectation du buffer courant
    glBindBuffer(GL_ARRAY_BUFFER, vbo);                                CHECK_GL_ERROR();
    //copie des donnees des sommets sur la carte graphique
    glBufferData(GL_ARRAY_BUFFER, sizeof(geometrie), geometrie, GL_STATIC_DRAW);  CHECK_GL_ERROR();

    // Active l'utilisation des données de positions (le 0 correspond à la location dans le vertex shader)
    glEnableVertexAttribArray(0); CHECK_GL_ERROR();
    // Indique comment le buffer courant (dernier vbo "bindé") est utilisé pour les positions des sommets
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(vertex_opengl), 0); CHECK_GL_ERROR();

    // Active l'utilisation des données de normales (le 1 correspond à la location dans le vertex shader)
    glEnableVertexAttribArray(1); CHECK_GL_ERROR();
    // Indique comment le buffer courant (dernier vbo "bindé") est utilisé pour les normales des sommets
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_TRUE, sizeof(vertex_opengl), (void*)sizeof(vec3)); CHECK_GL_ERROR();

    // Active l'utilisation des données de couleurs (le 2 correspond à la location dans le vertex shader)
    glEnableVertexAttribArray(2); CHECK_GL_ERROR();
    // Indique comment le buffer courant (dernier vbo "bindé") est utilisé pour les couleurs des sommets
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(vertex_opengl), (void*)(2 * sizeof(vec3))); CHECK_GL_ERROR();

    // Active l'utilisation des données de textures (le 3 correspond à la location dans le vertex shader)
    glEnableVertexAttribArray(3); CHECK_GL_ERROR();
    // Indique comment le buffer courant (dernier vbo "bindé") est utilisé pour les textures des sommets
    glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, sizeof(vertex_opengl), (void*)(3 * sizeof(vec3))); CHECK_GL_ERROR();

    GLuint vboi;
    //attribution d'un autre buffer de donnees
    glGenBuffers(1, &vboi);                                            CHECK_GL_ERROR();
    //affectation du buffer courant (buffer d'indice)
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vboi);                       CHECK_GL_ERROR();
    //copie des indices sur la carte graphique
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(index), index, GL_STATIC_DRAW);  CHECK_GL_ERROR();

    // Chargement de la texture
    load_texture("data/grass.tga", &model_sol.texture_id);

}


void init_model_3()
{
    // Chargement d'un maillage a partir d'un fichier
    mesh m = load_obj_file("data/missile2.obj");

    // Affecte une transformation sur les sommets du maillage
    float s = 0.2f;
    mat4 transform = mat4(s, 0.0f, 0.0f, 0.0f,
        0.0f, s, 0.0f, -0.9f,
        0.0f, 0.0f, s, -2.0f,
        0.0f, 0.0f, 0.0f, 1.0f);
    apply_deformation(&m, transform);

    // Centre la rotation du modele 1 autour de son centre de gravite approximatif
    model_missile.transformation_model.rotation_center = vec3(0.0f, -0.5f, -2.0f);

    // Calcul automatique des normales du maillage
    update_normals(&m);
    // Les sommets sont affectes a une couleur blanche
    fill_color(&m, vec3(1.0f, 1.0f, 1.0f));

    //attribution d'une liste d'état (1 indique la création d'une seule liste)
    glGenVertexArrays(1, &model_missile.vao);
    glBindVertexArray(model_missile.vao);

    GLuint vbo;
    //attribution d'un buffer de donnees (1 indique la création d'un buffer)
    glGenBuffers(1, &vbo); CHECK_GL_ERROR();
    //affectation du buffer courant
    glBindBuffer(GL_ARRAY_BUFFER, vbo); CHECK_GL_ERROR();
    //copie des donnees des sommets sur la carte graphique
    glBufferData(GL_ARRAY_BUFFER, m.vertex.size() * sizeof(vertex_opengl), &m.vertex[0], GL_STATIC_DRAW); CHECK_GL_ERROR();

    // Active l'utilisation des données de positions (le 0 correspond à la location dans le vertex shader)
    glEnableVertexAttribArray(0); CHECK_GL_ERROR();
    // Indique comment le buffer courant (dernier vbo "bindé") est utilisé pour les positions des sommets
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(vertex_opengl), 0); CHECK_GL_ERROR();

    // Active l'utilisation des données de normales (le 1 correspond à la location dans le vertex shader)
    glEnableVertexAttribArray(1); CHECK_GL_ERROR();
    // Indique comment le buffer courant (dernier vbo "bindé") est utilisé pour les normales des sommets
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_TRUE, sizeof(vertex_opengl), (void*)sizeof(vec3)); CHECK_GL_ERROR();

    // Active l'utilisation des données de couleurs (le 2 correspond à la location dans le vertex shader)
    glEnableVertexAttribArray(2); CHECK_GL_ERROR();
    // Indique comment le buffer courant (dernier vbo "bindé") est utilisé pour les couleurs des sommets
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(vertex_opengl), (void*)(2 * sizeof(vec3))); CHECK_GL_ERROR();

    // Active l'utilisation des données de textures (le 3 correspond à la location dans le vertex shader)
    glEnableVertexAttribArray(3); CHECK_GL_ERROR();
    // Indique comment le buffer courant (dernier vbo "bindé") est utilisé pour les textures des sommets
    glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, sizeof(vertex_opengl), (void*)(3 * sizeof(vec3))); CHECK_GL_ERROR();

    GLuint vboi;
    //attribution d'un autre buffer de donnees
    glGenBuffers(1, &vboi); CHECK_GL_ERROR();
    //affectation du buffer courant (buffer d'indice)
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vboi); CHECK_GL_ERROR();
    //copie des indices sur la carte graphique
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, m.connectivity.size() * sizeof(triangle_index), &m.connectivity[0], GL_STATIC_DRAW); CHECK_GL_ERROR();

    // Nombre de triangles de l'objet 1
    model_missile.nbr_triangle = m.connectivity.size();

    // Chargement de la texture
    load_texture("data/white.tga", &model_missile.texture_id);
}


void init_model_4()
{
    // Chargement d'un maillage a partir d'un fichier
    mesh m = load_obj_file("data/low_poly_tree.obj");

    // Affecte une transformation sur les sommets du maillage
    float s = 0.2f;
    mat4 transform = mat4(s, 0.0f, 0.0f, 0.0f,
        0.0f, s, 0.0f, -0.9f,
        0.0f, 0.0f, s, -2.0f,
        0.0f, 0.0f, 0.0f, 1.0f);
    apply_deformation(&m, transform);

    // Centre la rotation du modele 1 autour de son centre de gravite approximatif
    model_arbre.transformation_model.rotation_center = vec3(0.0f, -0.5f, -2.0f);

    // Calcul automatique des normales du maillage
    update_normals(&m);
    // Les sommets sont affectes a une couleur blanche
    fill_color(&m, vec3(1.0f, 1.0f, 1.0f));

    //attribution d'une liste d'état (1 indique la création d'une seule liste)
    glGenVertexArrays(1, &model_arbre.vao);
    glBindVertexArray(model_arbre.vao);

    GLuint vbo;
    //attribution d'un buffer de donnees (1 indique la création d'un buffer)
    glGenBuffers(1, &vbo); CHECK_GL_ERROR();
    //affectation du buffer courant
    glBindBuffer(GL_ARRAY_BUFFER, vbo); CHECK_GL_ERROR();
    //copie des donnees des sommets sur la carte graphique
    glBufferData(GL_ARRAY_BUFFER, m.vertex.size() * sizeof(vertex_opengl), &m.vertex[0], GL_STATIC_DRAW); CHECK_GL_ERROR();

    // Active l'utilisation des données de positions (le 0 correspond à la location dans le vertex shader)
    glEnableVertexAttribArray(0); CHECK_GL_ERROR();
    // Indique comment le buffer courant (dernier vbo "bindé") est utilisé pour les positions des sommets
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(vertex_opengl), 0); CHECK_GL_ERROR();

    // Active l'utilisation des données de normales (le 1 correspond à la location dans le vertex shader)
    glEnableVertexAttribArray(1); CHECK_GL_ERROR();
    // Indique comment le buffer courant (dernier vbo "bindé") est utilisé pour les normales des sommets
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_TRUE, sizeof(vertex_opengl), (void*)sizeof(vec3)); CHECK_GL_ERROR();

    // Active l'utilisation des données de couleurs (le 2 correspond à la location dans le vertex shader)
    glEnableVertexAttribArray(2); CHECK_GL_ERROR();
    // Indique comment le buffer courant (dernier vbo "bindé") est utilisé pour les couleurs des sommets
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(vertex_opengl), (void*)(2 * sizeof(vec3))); CHECK_GL_ERROR();

    // Active l'utilisation des données de textures (le 3 correspond à la location dans le vertex shader)
    glEnableVertexAttribArray(3); CHECK_GL_ERROR();
    // Indique comment le buffer courant (dernier vbo "bindé") est utilisé pour les textures des sommets
    glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, sizeof(vertex_opengl), (void*)(3 * sizeof(vec3))); CHECK_GL_ERROR();

    GLuint vboi;
    //attribution d'un autre buffer de donnees
    glGenBuffers(1, &vboi); CHECK_GL_ERROR();
    //affectation du buffer courant (buffer d'indice)
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vboi); CHECK_GL_ERROR();
    //copie des indices sur la carte graphique
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, m.connectivity.size() * sizeof(triangle_index), &m.connectivity[0], GL_STATIC_DRAW); CHECK_GL_ERROR();

    // Nombre de triangles de l'objet 1
    model_arbre.nbr_triangle = m.connectivity.size();

    // Chargement de la texture
    load_texture("data/green.tga", &model_arbre.texture_id);
}



void init_model_5()
{
    // Chargement d'un maillage a partir d'un fichier
    mesh m = load_obj_file("data/gagne.obj");

    // Affecte une transformation sur les sommets du maillage
    float s = 0.2f;
    mat4 transform = mat4(s, 0.0f, 0.0f, 0.0f,
        0.0f, s, 0.0f, -0.9f,
        0.0f, 0.0f, s, -2.0f,
        0.0f, 0.0f, 0.0f, 1.0f);
    apply_deformation(&m, transform);

    // Centre la rotation du modele 1 autour de son centre de gravite approximatif
    model_gagne.transformation_model.rotation_center = vec3(0.0f, -0.5f, -2.0f);

    // Calcul automatique des normales du maillage
    update_normals(&m);
    // Les sommets sont affectes a une couleur blanche
    fill_color(&m, vec3(1.0f, 1.0f, 1.0f));

    //attribution d'une liste d'état (1 indique la création d'une seule liste)
    glGenVertexArrays(1, &model_gagne.vao);
    glBindVertexArray(model_gagne.vao);

    GLuint vbo;
    //attribution d'un buffer de donnees (1 indique la création d'un buffer)
    glGenBuffers(1, &vbo); CHECK_GL_ERROR();
    //affectation du buffer courant
    glBindBuffer(GL_ARRAY_BUFFER, vbo); CHECK_GL_ERROR();
    //copie des donnees des sommets sur la carte graphique
    glBufferData(GL_ARRAY_BUFFER, m.vertex.size() * sizeof(vertex_opengl), &m.vertex[0], GL_STATIC_DRAW); CHECK_GL_ERROR();

    // Active l'utilisation des données de positions (le 0 correspond à la location dans le vertex shader)
    glEnableVertexAttribArray(0); CHECK_GL_ERROR();
    // Indique comment le buffer courant (dernier vbo "bindé") est utilisé pour les positions des sommets
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(vertex_opengl), 0); CHECK_GL_ERROR();

    // Active l'utilisation des données de normales (le 1 correspond à la location dans le vertex shader)
    glEnableVertexAttribArray(1); CHECK_GL_ERROR();
    // Indique comment le buffer courant (dernier vbo "bindé") est utilisé pour les normales des sommets
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_TRUE, sizeof(vertex_opengl), (void*)sizeof(vec3)); CHECK_GL_ERROR();

    // Active l'utilisation des données de couleurs (le 2 correspond à la location dans le vertex shader)
    glEnableVertexAttribArray(2); CHECK_GL_ERROR();
    // Indique comment le buffer courant (dernier vbo "bindé") est utilisé pour les couleurs des sommets
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(vertex_opengl), (void*)(2 * sizeof(vec3))); CHECK_GL_ERROR();

    // Active l'utilisation des données de textures (le 3 correspond à la location dans le vertex shader)
    glEnableVertexAttribArray(3); CHECK_GL_ERROR();
    // Indique comment le buffer courant (dernier vbo "bindé") est utilisé pour les textures des sommets
    glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, sizeof(vertex_opengl), (void*)(3 * sizeof(vec3))); CHECK_GL_ERROR();

    GLuint vboi;
    //attribution d'un autre buffer de donnees
    glGenBuffers(1, &vboi); CHECK_GL_ERROR();
    //affectation du buffer courant (buffer d'indice)
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vboi); CHECK_GL_ERROR();
    //copie des indices sur la carte graphique
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, m.connectivity.size() * sizeof(triangle_index), &m.connectivity[0], GL_STATIC_DRAW); CHECK_GL_ERROR();

    // Nombre de triangles de l'objet 1
    model_gagne.nbr_triangle = m.connectivity.size();

    // Chargement de la texture
    load_texture("data/white.tga", &model_gagne.texture_id);
}




void init_model_6()
{
    // Chargement d'un maillage a partir d'un fichier
    mesh m = load_obj_file("data/perdu.obj");

    // Affecte une transformation sur les sommets du maillage
    float s = 0.2f;
    mat4 transform = mat4(s, 0.0f, 0.0f, 0.0f,
        0.0f, s, 0.0f, -0.9f,
        0.0f, 0.0f, s, -2.0f,
        0.0f, 0.0f, 0.0f, 1.0f);
    apply_deformation(&m, transform);

    // Centre la rotation du modele 1 autour de son centre de gravite approximatif
    model_perdu.transformation_model.rotation_center = vec3(0.0f, -0.5f, -2.0f);

    // Calcul automatique des normales du maillage
    update_normals(&m);
    // Les sommets sont affectes a une couleur blanche
    fill_color(&m, vec3(1.0f, 1.0f, 1.0f));

    //attribution d'une liste d'état (1 indique la création d'une seule liste)
    glGenVertexArrays(1, &model_perdu.vao);
    glBindVertexArray(model_perdu.vao);

    GLuint vbo;
    //attribution d'un buffer de donnees (1 indique la création d'un buffer)
    glGenBuffers(1, &vbo); CHECK_GL_ERROR();
    //affectation du buffer courant
    glBindBuffer(GL_ARRAY_BUFFER, vbo); CHECK_GL_ERROR();
    //copie des donnees des sommets sur la carte graphique
    glBufferData(GL_ARRAY_BUFFER, m.vertex.size() * sizeof(vertex_opengl), &m.vertex[0], GL_STATIC_DRAW); CHECK_GL_ERROR();

    // Active l'utilisation des données de positions (le 0 correspond à la location dans le vertex shader)
    glEnableVertexAttribArray(0); CHECK_GL_ERROR();
    // Indique comment le buffer courant (dernier vbo "bindé") est utilisé pour les positions des sommets
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(vertex_opengl), 0); CHECK_GL_ERROR();

    // Active l'utilisation des données de normales (le 1 correspond à la location dans le vertex shader)
    glEnableVertexAttribArray(1); CHECK_GL_ERROR();
    // Indique comment le buffer courant (dernier vbo "bindé") est utilisé pour les normales des sommets
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_TRUE, sizeof(vertex_opengl), (void*)sizeof(vec3)); CHECK_GL_ERROR();

    // Active l'utilisation des données de couleurs (le 2 correspond à la location dans le vertex shader)
    glEnableVertexAttribArray(2); CHECK_GL_ERROR();
    // Indique comment le buffer courant (dernier vbo "bindé") est utilisé pour les couleurs des sommets
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(vertex_opengl), (void*)(2 * sizeof(vec3))); CHECK_GL_ERROR();

    // Active l'utilisation des données de textures (le 3 correspond à la location dans le vertex shader)
    glEnableVertexAttribArray(3); CHECK_GL_ERROR();
    // Indique comment le buffer courant (dernier vbo "bindé") est utilisé pour les textures des sommets
    glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, sizeof(vertex_opengl), (void*)(3 * sizeof(vec3))); CHECK_GL_ERROR();

    GLuint vboi;
    //attribution d'un autre buffer de donnees
    glGenBuffers(1, &vboi); CHECK_GL_ERROR();
    //affectation du buffer courant (buffer d'indice)
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vboi); CHECK_GL_ERROR();
    //copie des indices sur la carte graphique
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, m.connectivity.size() * sizeof(triangle_index), &m.connectivity[0], GL_STATIC_DRAW); CHECK_GL_ERROR();

    // Nombre de triangles de l'objet 1
    model_perdu.nbr_triangle = m.connectivity.size();

    // Chargement de la texture
    load_texture("data/white.tga", &model_perdu.texture_id);
}


void load_texture(const char* filename, GLuint* texture_id)
{
    // Chargement d'une texture (seul les textures tga sont supportes)
    Image* image = image_load_tga(filename);
    if (image) //verification que l'image est bien chargee
    {

        //Creation d'un identifiant pour la texture
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1); CHECK_GL_ERROR();
        glGenTextures(1, texture_id); CHECK_GL_ERROR();

        //Selection de la texture courante a partir de son identifiant
        glBindTexture(GL_TEXTURE_2D, *texture_id); CHECK_GL_ERROR();

        //Parametres de la texture
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT); CHECK_GL_ERROR();
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT); CHECK_GL_ERROR();
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR); CHECK_GL_ERROR();
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR); CHECK_GL_ERROR();

        //Envoie de l'image en memoire video
        if (image->type == IMAGE_TYPE_RGB) { //image RGB
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, image->width, image->height, 0, GL_RGB, GL_UNSIGNED_BYTE, image->data); CHECK_GL_ERROR();
        }
        else if (image->type == IMAGE_TYPE_RGBA) { //image RGBA
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image->width, image->height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image->data); CHECK_GL_ERROR();
        }
        else {
            std::cout << "Image type not handled" << std::endl;
        }

        delete image;
    }
    else
    {
        std::cerr << "Erreur chargement de l'image, etes-vous dans le bon repertoire?" << std::endl;
        abort();
    }

    GLint loc_texture = glGetUniformLocation(shader_program_id, "texture"); CHECK_GL_ERROR();
    if (loc_texture == -1) std::cerr << "Pas de variable uniforme : texture" << std::endl;
    glUniform1i(loc_texture, 0); CHECK_GL_ERROR();
}
