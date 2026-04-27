// =============================================================================
// PEA-UPC: Programa Estadístico de Análisis (versión C++)
// Universidad Popular del Cesar - Estructura de Datos
// Autor: [Tu nombre aquí]
// =============================================================================
// Este programa gestiona la información de investigación (grupos,
// investigadores y productos) descargada desde SCIENTI de MinCiencias.
//
// TODA la información se almacena en LISTAS (std::vector) de C++.
// Cada elemento de la lista es una struct con los campos del registro.
//
// ESTRUCTURA DE LOS DATOS:
//   grupos         = vector de grupos de investigación
//   investigadores = vector de investigadores (profesores)
//   productos      = vector de productos (artículos, libros, tesis, etc.)
//
// VARIABLES DE ENTRADA / SALIDA:
//   Grupo:         codigo, nombre, lider, categoria, area, institucion,
//                  anio_formacion  -->  estado, productos, integrantes
//   Investigador:  cvlac, nombre, categoria, formacion, grupo_codigo
//                  -->  estado, productos
//   Producto:      id, tipo, titulo, anio, autores, categoria,
//                  grupo_codigo, investigador_cvlac  -->  estado, validado
//
// COMPILACIÓN:   g++ -std=c++17 pea_upc.cpp -o pea_upc
// EJECUCIÓN:     ./pea_upc       (Linux/Mac)
//                pea_upc.exe     (Windows)
// =============================================================================

#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <ctime>
#include <algorithm>
#include <map>
#include <sys/stat.h>

#ifdef _WIN32
    #include <direct.h>
    #define MAKE_DIR(x) _mkdir(x)
#else
    #include <sys/types.h>
    #define MAKE_DIR(x) mkdir(x, 0755)
#endif

using namespace std;


// =============================================================================
// ESTRUCTURAS (los "registros" del modelo)
// =============================================================================
struct Grupo {
    string codigo;
    string nombre;
    string lider;
    string categoria;
    string area;
    string institucion;
    int    anio_formacion;
    string estado;                    // "activo" o "inactivo"
    vector<string> integrantes;       // CvLACs de los profesores
    vector<string> productos;         // IDs de productos del grupo
};

struct Investigador {
    string cvlac;
    string nombre;
    string categoria;
    string formacion;
    string grupo_codigo;
    string estado;
    vector<string> productos;         // IDs de productos del investigador
};

struct Producto {
    string id;
    string tipo;
    string titulo;
    int    anio;
    vector<string> autores;
    string categoria;
    string grupo_codigo;
    string investigador_cvlac;
    bool   validado;
    string estado;
};


// =============================================================================
// LISTAS PRINCIPALES (toda la información vive aquí)
// =============================================================================
vector<Grupo>        grupos;
vector<Investigador> investigadores;
vector<Producto>     productos;


// =============================================================================
// CONSTANTES: opciones válidas de MinCiencias
// =============================================================================
const vector<string> CATEGORIAS_GRUPO =
    {"A1", "A", "B", "C", "Reconocido", "Registrado"};

const vector<string> CATEGORIAS_INVESTIGADOR =
    {"Emerito", "Senior", "Asociado", "Junior"};

const vector<string> CATEGORIAS_PRODUCTO =
    {"A1", "A", "B", "C", "D", "Sin categoria"};

const vector<string> TIPOS_PRODUCTO = {
    "Articulo de investigacion",
    "Libro resultado de investigacion",
    "Capitulo de libro",
    "Software",
    "Tesis de maestria dirigida",
    "Tesis de doctorado dirigida",
    "Ponencia",
    "Producto tecnologico"
};

const vector<string> NIVELES_FORMACION =
    {"Doctorado", "Maestria", "Especializacion", "Pregrado"};


// =============================================================================
// FUNCIONES AUXILIARES: validar la entrada del usuario
// =============================================================================
string pedir_texto(const string& mensaje) {
    string texto;
    while (true) {
        cout << mensaje;
        getline(cin, texto);
        // Quitar espacios al inicio y al final
        size_t a = texto.find_first_not_of(" \t");
        size_t b = texto.find_last_not_of(" \t");
        if (a != string::npos)
            texto = texto.substr(a, b - a + 1);
        else
            texto = "";

        if (!texto.empty())
            return texto;
        cout << "  [!] No puede estar vacio." << endl;
    }
}

int pedir_entero(const string& mensaje, int minimo, int maximo) {
    string entrada;
    while (true) {
        cout << mensaje;
        getline(cin, entrada);
        try {
            int valor = stoi(entrada);
            if (valor >= minimo && valor <= maximo)
                return valor;
            cout << "  [!] Debe estar entre " << minimo
                 << " y " << maximo << "." << endl;
        } catch (...) {
            cout << "  [!] Debes ingresar un numero entero." << endl;
        }
    }
}

bool pedir_si_no(const string& mensaje) {
    string r;
    while (true) {
        cout << mensaje << " (s/n): ";
        getline(cin, r);
        if (r == "s" || r == "S") return true;
        if (r == "n" || r == "N") return false;
        cout << "  [!] Responde 's' o 'n'." << endl;
    }
}

string pedir_opcion(const string& mensaje, const vector<string>& opciones) {
    cout << mensaje << endl;
    for (size_t i = 0; i < opciones.size(); i++) {
        cout << "   " << (i + 1) << ". " << opciones[i] << endl;
    }
    int num = pedir_entero("Elige el numero: ", 1, (int)opciones.size());
    return opciones[num - 1];
}


// =============================================================================
// FUNCIONES DE BÚSQUEDA
// =============================================================================
int buscar_grupo(const string& codigo) {
    for (size_t i = 0; i < grupos.size(); i++) {
        if (grupos[i].codigo == codigo)
            return (int)i;
    }
    return -1;
}

int buscar_investigador(const string& cvlac) {
    for (size_t i = 0; i < investigadores.size(); i++) {
        if (investigadores[i].cvlac == cvlac)
            return (int)i;
    }
    return -1;
}

int buscar_producto(const string& id_prod) {
    for (size_t i = 0; i < productos.size(); i++) {
        if (productos[i].id == id_prod)
            return (int)i;
    }
    return -1;
}

string siguiente_id_producto() {
    if (productos.empty())
        return "P0001";
    int mayor = 0;
    for (const auto& p : productos) {
        if (p.id.size() > 1) {
            try {
                int n = stoi(p.id.substr(1));
                if (n > mayor) mayor = n;
            } catch (...) {}
        }
    }
    int siguiente = mayor + 1;
    string num = to_string(siguiente);
    while (num.size() < 4) num = "0" + num;
    return "P" + num;
}


// =============================================================================
// FUNCIONES UTILITARIAS PARA STRINGS
// =============================================================================
vector<string> dividir(const string& texto, char separador) {
    vector<string> partes;
    string actual;
    for (char c : texto) {
        if (c == separador) {
            if (!actual.empty()) partes.push_back(actual);
            actual.clear();
        } else {
            actual += c;
        }
    }
    if (!actual.empty()) partes.push_back(actual);
    return partes;
}

string unir(const vector<string>& partes, const string& separador) {
    string resultado;
    for (size_t i = 0; i < partes.size(); i++) {
        if (i > 0) resultado += separador;
        resultado += partes[i];
    }
    return resultado;
}

bool contiene(const vector<string>& lista, const string& valor) {
    for (const auto& v : lista) {
        if (v == valor) return true;
    }
    return false;
}

void quitar(vector<string>& lista, const string& valor) {
    lista.erase(remove(lista.begin(), lista.end(), valor), lista.end());
}


// =============================================================================
// GESTIÓN DE GRUPOS
// =============================================================================
void incluir_grupo() {
    cout << "\n>> Nuevo grupo de investigacion" << endl;
    string codigo = pedir_texto("Codigo del grupo (ej: COL0001): ");
    if (buscar_grupo(codigo) != -1) {
        cout << "  [!] Ya existe un grupo con ese codigo." << endl;
        return;
    }

    Grupo g;
    g.codigo = codigo;
    g.nombre = pedir_texto("Nombre del grupo: ");
    g.lider = pedir_texto("Nombre del lider: ");
    g.categoria = pedir_opcion("Categoria:", CATEGORIAS_GRUPO);
    g.area = pedir_texto("Area de conocimiento: ");
    g.institucion = pedir_texto("Institucion: ");
    g.anio_formacion = pedir_entero("Año de formacion: ", 1900, 2100);
    g.estado = "activo";

    grupos.push_back(g);
    cout << "  [OK] Grupo '" << g.nombre << "' creado." << endl;
}

void listar_grupos() {
    cout << "\n--- GRUPOS ---" << endl;
    if (grupos.empty()) {
        cout << "  (sin grupos)" << endl;
        return;
    }
    cout << "Codigo       Nombre                         Cat    Estado" << endl;
    cout << "-----------------------------------------------------------------" << endl;
    for (const auto& g : grupos) {
        string nombre = g.nombre.substr(0, 30);
        while (nombre.size() < 30) nombre += " ";
        string cat = g.categoria;
        while (cat.size() < 6) cat += " ";
        string codigo = g.codigo;
        while (codigo.size() < 12) codigo += " ";
        cout << codigo << " " << nombre << " " << cat << " " << g.estado << endl;
    }
}

void ver_grupo() {
    string codigo = pedir_texto("Codigo del grupo: ");
    int i = buscar_grupo(codigo);
    if (i == -1) {
        cout << "  [!] No existe ese grupo." << endl;
        return;
    }

    const Grupo& g = grupos[i];
    cout << "\n--- DETALLE DEL GRUPO " << g.codigo << " ---" << endl;
    cout << "  Nombre:       " << g.nombre << endl;
    cout << "  Lider:        " << g.lider << endl;
    cout << "  Categoria:    " << g.categoria << endl;
    cout << "  Area:         " << g.area << endl;
    cout << "  Institucion:  " << g.institucion << endl;
    cout << "  Año:          " << g.anio_formacion << endl;
    cout << "  Estado:       " << g.estado << endl;

    cout << "\n  Profesores (" << g.integrantes.size() << "):" << endl;
    for (const auto& cvlac : g.integrantes) {
        int j = buscar_investigador(cvlac);
        if (j != -1)
            cout << "    - " << cvlac << ": "
                 << investigadores[j].nombre << endl;
    }

    cout << "\n  Productos (" << g.productos.size() << "):" << endl;
    for (const auto& id_p : g.productos) {
        int k = buscar_producto(id_p);
        if (k != -1) {
            const Producto& p = productos[k];
            cout << "    - " << p.id << ": "
                 << p.titulo.substr(0, 50) << " (" << p.anio << ")" << endl;
        }
    }
}

void modificar_grupo() {
    string codigo = pedir_texto("Codigo del grupo a modificar: ");
    int i = buscar_grupo(codigo);
    if (i == -1) {
        cout << "  [!] No existe ese grupo." << endl;
        return;
    }

    Grupo& g = grupos[i];
    cout << "\n¿Que quieres modificar?" << endl;
    cout << "   1. Nombre" << endl;
    cout << "   2. Lider" << endl;
    cout << "   3. Categoria" << endl;
    cout << "   4. Area" << endl;
    cout << "   5. Institucion" << endl;
    cout << "   6. Año de formacion" << endl;
    int op = pedir_entero("Opcion: ", 1, 6);

    if      (op == 1) g.nombre = pedir_texto("Nuevo nombre: ");
    else if (op == 2) g.lider = pedir_texto("Nuevo lider: ");
    else if (op == 3) g.categoria = pedir_opcion("Nueva categoria:", CATEGORIAS_GRUPO);
    else if (op == 4) g.area = pedir_texto("Nueva area: ");
    else if (op == 5) g.institucion = pedir_texto("Nueva institucion: ");
    else if (op == 6) g.anio_formacion = pedir_entero("Nuevo año: ", 1900, 2100);
    cout << "  [OK] Grupo modificado." << endl;
}

void desactivar_grupo() {
    string codigo = pedir_texto("Codigo del grupo: ");
    int i = buscar_grupo(codigo);
    if (i == -1) { cout << "  [!] No existe." << endl; return; }
    grupos[i].estado = "inactivo";
    cout << "  [OK] Grupo desactivado." << endl;
}

void reactivar_grupo() {
    string codigo = pedir_texto("Codigo del grupo: ");
    int i = buscar_grupo(codigo);
    if (i == -1) { cout << "  [!] No existe." << endl; return; }
    grupos[i].estado = "activo";
    cout << "  [OK] Grupo reactivado." << endl;
}

void eliminar_grupo() {
    string codigo = pedir_texto("Codigo del grupo a eliminar: ");
    int i = buscar_grupo(codigo);
    if (i == -1) { cout << "  [!] No existe." << endl; return; }
    if (!pedir_si_no("¿Seguro que quieres eliminar '" + grupos[i].nombre + "'?")) {
        cout << "  Cancelado." << endl;
        return;
    }

    // Limpiar referencias en investigadores y productos
    for (auto& inv : investigadores) {
        if (inv.grupo_codigo == codigo)
            inv.grupo_codigo = "";
    }
    for (auto& p : productos) {
        if (p.grupo_codigo == codigo)
            p.grupo_codigo = "";
    }

    grupos.erase(grupos.begin() + i);
    cout << "  [OK] Grupo eliminado." << endl;
}

void agregar_profesor_a_grupo() {
    string codigo = pedir_texto("Codigo del grupo: ");
    int i = buscar_grupo(codigo);
    if (i == -1) { cout << "  [!] Grupo no existe." << endl; return; }
    string cvlac = pedir_texto("CvLAC del profesor: ");
    int j = buscar_investigador(cvlac);
    if (j == -1) { cout << "  [!] Investigador no existe." << endl; return; }

    if (!contiene(grupos[i].integrantes, cvlac))
        grupos[i].integrantes.push_back(cvlac);
    investigadores[j].grupo_codigo = codigo;
    cout << "  [OK] Profesor agregado al grupo." << endl;
}


// =============================================================================
// GESTIÓN DE INVESTIGADORES
// =============================================================================
void incluir_investigador() {
    cout << "\n>> Nuevo investigador" << endl;
    string cvlac = pedir_texto("Codigo CvLAC: ");
    if (buscar_investigador(cvlac) != -1) {
        cout << "  [!] Ya existe ese investigador." << endl;
        return;
    }

    Investigador inv;
    inv.cvlac = cvlac;
    inv.nombre = pedir_texto("Nombre completo: ");
    inv.categoria = pedir_opcion("Categoria:", CATEGORIAS_INVESTIGADOR);
    inv.formacion = pedir_opcion("Formacion:", NIVELES_FORMACION);
    inv.grupo_codigo = "";
    inv.estado = "activo";

    if (pedir_si_no("¿Asignar a un grupo ahora?")) {
        string codigo = pedir_texto("Codigo del grupo: ");
        int i = buscar_grupo(codigo);
        if (i == -1) {
            cout << "  [!] Grupo no existe. Se crea sin grupo." << endl;
        } else {
            inv.grupo_codigo = codigo;
            if (!contiene(grupos[i].integrantes, cvlac))
                grupos[i].integrantes.push_back(cvlac);
        }
    }

    investigadores.push_back(inv);
    cout << "  [OK] Investigador '" << inv.nombre << "' creado." << endl;
}

void listar_investigadores() {
    cout << "\n--- INVESTIGADORES ---" << endl;
    if (investigadores.empty()) {
        cout << "  (sin investigadores)" << endl;
        return;
    }
    cout << "CvLAC        Nombre                    Cat        Grupo        Estado" << endl;
    cout << "----------------------------------------------------------------------" << endl;
    for (const auto& inv : investigadores) {
        string nom = inv.nombre.substr(0, 25);
        while (nom.size() < 25) nom += " ";
        string cat = inv.categoria;
        while (cat.size() < 10) cat += " ";
        string cv = inv.cvlac;
        while (cv.size() < 12) cv += " ";
        string gr = inv.grupo_codigo;
        while (gr.size() < 12) gr += " ";
        cout << cv << " " << nom << " " << cat << " " << gr << " " << inv.estado << endl;
    }
}

void ver_investigador() {
    string cvlac = pedir_texto("CvLAC: ");
    int i = buscar_investigador(cvlac);
    if (i == -1) { cout << "  [!] No existe." << endl; return; }

    const Investigador& inv = investigadores[i];
    cout << "\n--- DETALLE DEL INVESTIGADOR ---" << endl;
    cout << "  CvLAC:      " << inv.cvlac << endl;
    cout << "  Nombre:     " << inv.nombre << endl;
    cout << "  Categoria:  " << inv.categoria << endl;
    cout << "  Formacion:  " << inv.formacion << endl;
    cout << "  Grupo:      " << inv.grupo_codigo << endl;
    cout << "  Estado:     " << inv.estado << endl;

    cout << "\n  Productos (" << inv.productos.size() << "):" << endl;
    for (const auto& id_p : inv.productos) {
        int k = buscar_producto(id_p);
        if (k != -1) {
            const Producto& p = productos[k];
            cout << "    - " << p.id << ": "
                 << p.titulo.substr(0, 50) << " (" << p.anio << ")" << endl;
        }
    }
}

void modificar_investigador() {
    string cvlac = pedir_texto("CvLAC: ");
    int i = buscar_investigador(cvlac);
    if (i == -1) { cout << "  [!] No existe." << endl; return; }

    Investigador& inv = investigadores[i];
    cout << "\n¿Que quieres modificar?" << endl;
    cout << "   1. Nombre" << endl;
    cout << "   2. Categoria" << endl;
    cout << "   3. Formacion" << endl;
    cout << "   4. Grupo" << endl;
    int op = pedir_entero("Opcion: ", 1, 4);

    if      (op == 1) inv.nombre = pedir_texto("Nuevo nombre: ");
    else if (op == 2) inv.categoria = pedir_opcion("Categoria:", CATEGORIAS_INVESTIGADOR);
    else if (op == 3) inv.formacion = pedir_opcion("Formacion:", NIVELES_FORMACION);
    else if (op == 4) inv.grupo_codigo = pedir_texto("Codigo del grupo: ");
    cout << "  [OK] Investigador modificado." << endl;
}

void desactivar_investigador() {
    string cvlac = pedir_texto("CvLAC: ");
    int i = buscar_investigador(cvlac);
    if (i == -1) { cout << "  [!] No existe." << endl; return; }
    investigadores[i].estado = "inactivo";
    cout << "  [OK] Desactivado." << endl;
}

void reactivar_investigador() {
    string cvlac = pedir_texto("CvLAC: ");
    int i = buscar_investigador(cvlac);
    if (i == -1) { cout << "  [!] No existe." << endl; return; }
    investigadores[i].estado = "activo";
    cout << "  [OK] Reactivado." << endl;
}

void eliminar_investigador() {
    string cvlac = pedir_texto("CvLAC a eliminar: ");
    int i = buscar_investigador(cvlac);
    if (i == -1) { cout << "  [!] No existe." << endl; return; }
    if (!pedir_si_no("¿Eliminar '" + investigadores[i].nombre + "'?")) {
        cout << "  Cancelado." << endl;
        return;
    }

    // Quitar de los grupos
    for (auto& g : grupos)
        quitar(g.integrantes, cvlac);
    // Quitar de los productos
    for (auto& p : productos)
        if (p.investigador_cvlac == cvlac)
            p.investigador_cvlac = "";

    investigadores.erase(investigadores.begin() + i);
    cout << "  [OK] Eliminado." << endl;
}


// =============================================================================
// GESTIÓN DE PRODUCTOS
// =============================================================================
void incluir_producto() {
    cout << "\n>> Nuevo producto de investigacion" << endl;

    Producto p;
    p.id = siguiente_id_producto();
    p.tipo = pedir_opcion("Tipo de producto:", TIPOS_PRODUCTO);
    p.titulo = pedir_texto("Titulo: ");
    p.anio = pedir_entero("Año: ", 1900, 2100);

    string autores_txt = pedir_texto("Autores (separados por coma): ");
    p.autores = dividir(autores_txt, ',');

    p.categoria = pedir_opcion("Categoria MinCiencias:", CATEGORIAS_PRODUCTO);
    p.grupo_codigo = "";
    p.investigador_cvlac = "";

    if (pedir_si_no("¿Asociar a un grupo?")) {
        string codigo = pedir_texto("Codigo del grupo: ");
        if (buscar_grupo(codigo) == -1) {
            cout << "  [!] Grupo no existe. Se crea sin grupo." << endl;
        } else {
            p.grupo_codigo = codigo;
        }
    }

    if (pedir_si_no("¿Asociar a un investigador?")) {
        string cvlac = pedir_texto("CvLAC: ");
        if (buscar_investigador(cvlac) == -1) {
            cout << "  [!] Investigador no existe. Se crea sin investigador." << endl;
        } else {
            p.investigador_cvlac = cvlac;
        }
    }

    p.validado = pedir_si_no("¿Producto validado por el comite?");
    p.estado = "activo";

    productos.push_back(p);

    // Mantener relaciones bidireccionales
    if (!p.grupo_codigo.empty()) {
        int i = buscar_grupo(p.grupo_codigo);
        if (i != -1 && !contiene(grupos[i].productos, p.id))
            grupos[i].productos.push_back(p.id);
    }
    if (!p.investigador_cvlac.empty()) {
        int j = buscar_investigador(p.investigador_cvlac);
        if (j != -1 && !contiene(investigadores[j].productos, p.id))
            investigadores[j].productos.push_back(p.id);
    }

    cout << "  [OK] Producto '" << p.titulo
         << "' registrado con ID " << p.id << "." << endl;
}

void listar_productos() {
    cout << "\n--- PRODUCTOS ---" << endl;
    if (productos.empty()) {
        cout << "  (sin productos)" << endl;
        return;
    }
    cout << "ID      Titulo                                   Año    Cat          Val   Estado" << endl;
    cout << "-------------------------------------------------------------------------------------" << endl;
    for (const auto& p : productos) {
        string id = p.id;
        while (id.size() < 7) id += " ";
        string tit = p.titulo.substr(0, 40);
        while (tit.size() < 40) tit += " ";
        string cat = p.categoria;
        while (cat.size() < 12) cat += " ";
        string val = p.validado ? "Si " : "No ";
        cout << id << " " << tit << " " << p.anio << "   "
             << cat << " " << val << " " << p.estado << endl;
    }
}

void ver_producto() {
    string id_prod = pedir_texto("ID del producto: ");
    int i = buscar_producto(id_prod);
    if (i == -1) { cout << "  [!] No existe." << endl; return; }
    const Producto& p = productos[i];
    cout << "\n--- DETALLE DEL PRODUCTO " << p.id << " ---" << endl;
    cout << "  Tipo:        " << p.tipo << endl;
    cout << "  Titulo:      " << p.titulo << endl;
    cout << "  Año:         " << p.anio << endl;
    cout << "  Autores:     " << unir(p.autores, ", ") << endl;
    cout << "  Categoria:   " << p.categoria << endl;
    cout << "  Grupo:       " << p.grupo_codigo << endl;
    cout << "  Invest.:     " << p.investigador_cvlac << endl;
    cout << "  Validado:    " << (p.validado ? "Si" : "No") << endl;
    cout << "  Estado:      " << p.estado << endl;
}

void modificar_producto() {
    string id_prod = pedir_texto("ID del producto: ");
    int i = buscar_producto(id_prod);
    if (i == -1) { cout << "  [!] No existe." << endl; return; }

    Producto& p = productos[i];
    cout << "\n¿Que quieres modificar?" << endl;
    cout << "   1. Tipo" << endl;
    cout << "   2. Titulo" << endl;
    cout << "   3. Año" << endl;
    cout << "   4. Autores" << endl;
    cout << "   5. Categoria" << endl;
    cout << "   6. Validado (si/no)" << endl;
    int op = pedir_entero("Opcion: ", 1, 6);

    if      (op == 1) p.tipo = pedir_opcion("Tipo:", TIPOS_PRODUCTO);
    else if (op == 2) p.titulo = pedir_texto("Nuevo titulo: ");
    else if (op == 3) p.anio = pedir_entero("Nuevo año: ", 1900, 2100);
    else if (op == 4) {
        string txt = pedir_texto("Autores (separados por coma): ");
        p.autores = dividir(txt, ',');
    }
    else if (op == 5) p.categoria = pedir_opcion("Categoria:", CATEGORIAS_PRODUCTO);
    else if (op == 6) p.validado = pedir_si_no("¿Validado?");
    cout << "  [OK] Producto modificado." << endl;
}

void desactivar_producto() {
    string id_prod = pedir_texto("ID del producto: ");
    int i = buscar_producto(id_prod);
    if (i == -1) { cout << "  [!] No existe." << endl; return; }
    productos[i].estado = "inactivo";
    cout << "  [OK] Desactivado." << endl;
}

void reactivar_producto() {
    string id_prod = pedir_texto("ID del producto: ");
    int i = buscar_producto(id_prod);
    if (i == -1) { cout << "  [!] No existe." << endl; return; }
    productos[i].estado = "activo";
    cout << "  [OK] Reactivado." << endl;
}

void eliminar_producto() {
    string id_prod = pedir_texto("ID del producto: ");
    int i = buscar_producto(id_prod);
    if (i == -1) { cout << "  [!] No existe." << endl; return; }
    if (!pedir_si_no("¿Eliminar '" + productos[i].titulo + "'?")) {
        cout << "  Cancelado." << endl;
        return;
    }

    // Quitar referencias
    for (auto& g : grupos)
        quitar(g.productos, id_prod);
    for (auto& inv : investigadores)
        quitar(inv.productos, id_prod);

    productos.erase(productos.begin() + i);
    cout << "  [OK] Eliminado." << endl;
}


// =============================================================================
// FILTRAR PRODUCTOS POR AÑO
// =============================================================================
void filtrar_productos_por_anio() {
    cout << "\n>> Filtrar productos por año" << endl;
    cout << "   1. Ultimos 2 años" << endl;
    cout << "   2. Ultimos 5 años" << endl;
    cout << "   3. Ultimos 10 años" << endl;
    cout << "   4. Rango personalizado" << endl;
    int op = pedir_entero("Opcion: ", 1, 4);

    time_t t = time(nullptr);
    int anio_actual = localtime(&t)->tm_year + 1900;

    int desde, hasta;
    if      (op == 1) { desde = anio_actual - 1; hasta = anio_actual; }
    else if (op == 2) { desde = anio_actual - 4; hasta = anio_actual; }
    else if (op == 3) { desde = anio_actual - 9; hasta = anio_actual; }
    else {
        desde = pedir_entero("Año desde: ", 1900, anio_actual);
        hasta = pedir_entero("Año hasta: ", desde, anio_actual);
    }

    int total = 0;
    cout << "\n--- Productos entre " << desde << " y " << hasta << " ---" << endl;
    for (const auto& p : productos) {
        if (p.anio >= desde && p.anio <= hasta) {
            cout << "  " << p.id << ": " << p.titulo.substr(0, 50)
                 << " (" << p.anio << ") cat=" << p.categoria << endl;
            total++;
        }
    }
    cout << "Total: " << total << endl;
}


// =============================================================================
// PERSISTENCIA: guardar y cargar (formato CSV simple)
// =============================================================================
void asegurar_carpeta_datos() {
    struct stat info;
    if (stat("datos", &info) != 0) {
        MAKE_DIR("datos");
    }
}

// Reemplaza ';' por '|' en un texto para que no rompa el separador de campos
string escapar(const string& texto) {
    string r = texto;
    for (auto& c : r) if (c == ';') c = ',';
    return r;
}

void guardar_grupos() {
    asegurar_carpeta_datos();
    ofstream f("datos/grupos.csv");
    f << "codigo;nombre;lider;categoria;area;institucion;anio_formacion;estado;integrantes;productos\n";
    for (const auto& g : grupos) {
        f << g.codigo << ";"
          << escapar(g.nombre) << ";"
          << escapar(g.lider) << ";"
          << g.categoria << ";"
          << escapar(g.area) << ";"
          << escapar(g.institucion) << ";"
          << g.anio_formacion << ";"
          << g.estado << ";"
          << unir(g.integrantes, "|") << ";"
          << unir(g.productos, "|") << "\n";
    }
}

void guardar_investigadores() {
    asegurar_carpeta_datos();
    ofstream f("datos/investigadores.csv");
    f << "cvlac;nombre;categoria;formacion;grupo_codigo;estado;productos\n";
    for (const auto& inv : investigadores) {
        f << inv.cvlac << ";"
          << escapar(inv.nombre) << ";"
          << inv.categoria << ";"
          << inv.formacion << ";"
          << inv.grupo_codigo << ";"
          << inv.estado << ";"
          << unir(inv.productos, "|") << "\n";
    }
}

void guardar_productos() {
    asegurar_carpeta_datos();
    ofstream f("datos/productos.csv");
    f << "id;tipo;titulo;anio;autores;categoria;grupo_codigo;investigador_cvlac;validado;estado\n";
    for (const auto& p : productos) {
        f << p.id << ";"
          << escapar(p.tipo) << ";"
          << escapar(p.titulo) << ";"
          << p.anio << ";"
          << unir(p.autores, "|") << ";"
          << p.categoria << ";"
          << p.grupo_codigo << ";"
          << p.investigador_cvlac << ";"
          << (p.validado ? "true" : "false") << ";"
          << p.estado << "\n";
    }
}

void guardar_datos() {
    guardar_grupos();
    guardar_investigadores();
    guardar_productos();
    cout << "  [OK] Datos guardados en la carpeta 'datos/'." << endl;
}

// Lee una línea CSV respetando el separador
vector<string> parsear_linea(const string& linea, char sep) {
    vector<string> campos;
    string actual;
    for (char c : linea) {
        if (c == sep) {
            campos.push_back(actual);
            actual.clear();
        } else {
            actual += c;
        }
    }
    campos.push_back(actual);
    return campos;
}

void cargar_grupos() {
    ifstream f("datos/grupos.csv");
    if (!f.is_open()) return;
    string linea;
    getline(f, linea);  // saltar cabecera
    while (getline(f, linea)) {
        if (linea.empty()) continue;
        auto c = parsear_linea(linea, ';');
        if (c.size() < 10) continue;
        Grupo g;
        g.codigo = c[0];
        g.nombre = c[1];
        g.lider = c[2];
        g.categoria = c[3];
        g.area = c[4];
        g.institucion = c[5];
        try { g.anio_formacion = stoi(c[6]); } catch (...) { g.anio_formacion = 0; }
        g.estado = c[7];
        if (!c[8].empty()) g.integrantes = dividir(c[8], '|');
        if (!c[9].empty()) g.productos = dividir(c[9], '|');
        grupos.push_back(g);
    }
}

void cargar_investigadores() {
    ifstream f("datos/investigadores.csv");
    if (!f.is_open()) return;
    string linea;
    getline(f, linea);
    while (getline(f, linea)) {
        if (linea.empty()) continue;
        auto c = parsear_linea(linea, ';');
        if (c.size() < 7) continue;
        Investigador inv;
        inv.cvlac = c[0];
        inv.nombre = c[1];
        inv.categoria = c[2];
        inv.formacion = c[3];
        inv.grupo_codigo = c[4];
        inv.estado = c[5];
        if (!c[6].empty()) inv.productos = dividir(c[6], '|');
        investigadores.push_back(inv);
    }
}

void cargar_productos() {
    ifstream f("datos/productos.csv");
    if (!f.is_open()) return;
    string linea;
    getline(f, linea);
    while (getline(f, linea)) {
        if (linea.empty()) continue;
        auto c = parsear_linea(linea, ';');
        if (c.size() < 10) continue;
        Producto p;
        p.id = c[0];
        p.tipo = c[1];
        p.titulo = c[2];
        try { p.anio = stoi(c[3]); } catch (...) { p.anio = 0; }
        if (!c[4].empty()) p.autores = dividir(c[4], '|');
        p.categoria = c[5];
        p.grupo_codigo = c[6];
        p.investigador_cvlac = c[7];
        p.validado = (c[8] == "true" || c[8] == "1");
        p.estado = c[9];
        productos.push_back(p);
    }
}

void cargar_datos() {
    cargar_grupos();
    cargar_investigadores();
    cargar_productos();
}


// =============================================================================
// IMPORTAR PRODUCTOS DESDE CSV
// =============================================================================
void importar_csv() {
    string ruta = pedir_texto("Ruta del archivo CSV: ");
    // Quitar comillas si Windows las copió
    if (!ruta.empty() && ruta.front() == '"') ruta.erase(0, 1);
    if (!ruta.empty() && ruta.back() == '"') ruta.pop_back();

    ifstream f(ruta);
    if (!f.is_open()) {
        cout << "  [!] No se encontro el archivo." << endl;
        cout << "      Busque en: " << ruta << endl;
        return;
    }

    string primera;
    getline(f, primera);
    char sep = ';';
    int n_punto_coma = count(primera.begin(), primera.end(), ';');
    int n_coma = count(primera.begin(), primera.end(), ',');
    if (n_coma > n_punto_coma) sep = ',';
    cout << "  [i] Separador detectado: '" << sep << "'" << endl;

    auto cabecera = parsear_linea(primera, sep);
    map<string, int> indice;
    for (size_t i = 0; i < cabecera.size(); i++) {
        indice[cabecera[i]] = (int)i;
    }

    int anadidos = 0, saltados = 0;
    string linea;
    while (getline(f, linea)) {
        if (linea.empty()) continue;
        auto c = parsear_linea(linea, sep);

        auto getCampo = [&](const string& nombre) -> string {
            if (indice.count(nombre) && indice[nombre] < (int)c.size())
                return c[indice[nombre]];
            return "";
        };

        string id = getCampo("id");
        if (!id.empty() && buscar_producto(id) != -1) {
            saltados++;
            continue;
        }

        Producto p;
        p.id = id.empty() ? siguiente_id_producto() : id;
        p.tipo = getCampo("tipo").empty() ? "Articulo de investigacion" : getCampo("tipo");
        p.titulo = getCampo("titulo").empty() ? "(sin titulo)" : getCampo("titulo");
        try { p.anio = stoi(getCampo("anio")); } catch (...) { p.anio = 0; }

        string aut = getCampo("autores");
        char sep_aut = aut.find('|') != string::npos ? '|' : ',';
        if (!aut.empty()) p.autores = dividir(aut, sep_aut);

        p.categoria = getCampo("categoria").empty() ? "Sin categoria" : getCampo("categoria");
        p.grupo_codigo = getCampo("grupo_codigo");
        p.investigador_cvlac = getCampo("investigador_cvlac");
        string val = getCampo("validado");
        p.validado = (val == "true" || val == "1" || val == "si" || val == "Si");
        p.estado = "activo";

        productos.push_back(p);

        // Mantener relaciones
        if (!p.grupo_codigo.empty()) {
            int i = buscar_grupo(p.grupo_codigo);
            if (i != -1 && !contiene(grupos[i].productos, p.id))
                grupos[i].productos.push_back(p.id);
        }
        if (!p.investigador_cvlac.empty()) {
            int j = buscar_investigador(p.investigador_cvlac);
            if (j != -1 && !contiene(investigadores[j].productos, p.id))
                investigadores[j].productos.push_back(p.id);
        }
        anadidos++;
    }

    cout << "  [OK] Productos importados: " << anadidos << endl;
    if (saltados > 0)
        cout << "  [i] Productos saltados (ya existian): " << saltados << endl;
}


// =============================================================================
// RESUMEN ESTADÍSTICO (en C, esquema descriptivo según el enunciado)
// =============================================================================
void resumen_estadistico() {
    cout << "\n============================================================" << endl;
    cout << "   RESUMEN ESTADISTICO - PEA UPC" << endl;
    cout << "============================================================" << endl;

    // Totales
    int activos_g = 0, inactivos_g = 0;
    for (const auto& g : grupos) {
        if (g.estado == "activo") activos_g++;
        else inactivos_g++;
    }

    int activos_i = 0, inactivos_i = 0;
    for (const auto& inv : investigadores) {
        if (inv.estado == "activo") activos_i++;
        else inactivos_i++;
    }

    int validados = 0, no_validados = 0;
    for (const auto& p : productos) {
        if (p.validado) validados++;
        else no_validados++;
    }

    cout << "  Grupos:         " << grupos.size()
         << " (" << activos_g << " activos, " << inactivos_g << " inactivos)" << endl;
    cout << "  Investigadores: " << investigadores.size()
         << " (" << activos_i << " activos, " << inactivos_i << " inactivos)" << endl;
    cout << "  Productos:      " << productos.size()
         << " (" << validados << " validados, " << no_validados << " sin validar)" << endl;

    // Productos por año (últimos 10 años)
    time_t t = time(nullptr);
    int anio_actual = localtime(&t)->tm_year + 1900;
    map<int, int> conteo_anios;
    for (const auto& p : productos) {
        if (p.estado == "activo" && p.anio >= anio_actual - 9 && p.anio <= anio_actual) {
            conteo_anios[p.anio]++;
        }
    }

    if (!conteo_anios.empty()) {
        cout << "\n  Productos por año (ultimos 10 años):" << endl;
        for (const auto& par : conteo_anios) {
            cout << "    " << par.first << ": ";
            for (int i = 0; i < par.second; i++) cout << "#";
            cout << " (" << par.second << ")" << endl;
        }
    }

    // Productos por categoría
    map<string, int> conteo_cat;
    for (const auto& p : productos) {
        if (p.estado == "activo") conteo_cat[p.categoria]++;
    }

    if (!conteo_cat.empty()) {
        cout << "\n  Productos por categoria:" << endl;
        for (const auto& par : conteo_cat) {
            cout << "    " << par.first << ": " << par.second << endl;
        }
    }

    // Productos por tipo
    map<string, int> conteo_tipo;
    for (const auto& p : productos) {
        if (p.estado == "activo") conteo_tipo[p.tipo]++;
    }

    if (!conteo_tipo.empty()) {
        cout << "\n  Productos por tipo:" << endl;
        for (const auto& par : conteo_tipo) {
            cout << "    " << par.first << ": " << par.second << endl;
        }
    }

    cout << "============================================================" << endl;
}


// =============================================================================
// MENÚS
// =============================================================================
void menu_grupos() {
    while (true) {
        cout << "\n--- MENU GRUPOS ---" << endl;
        cout << "  1. Incluir grupo" << endl;
        cout << "  2. Listar grupos" << endl;
        cout << "  3. Ver detalle" << endl;
        cout << "  4. Modificar grupo" << endl;
        cout << "  5. Desactivar grupo" << endl;
        cout << "  6. Reactivar grupo" << endl;
        cout << "  7. Eliminar grupo" << endl;
        cout << "  8. Agregar profesor a un grupo" << endl;
        cout << "  0. Volver" << endl;
        cout << " Opcion: ";
        string op;
        getline(cin, op);

        if      (op == "1") incluir_grupo();
        else if (op == "2") listar_grupos();
        else if (op == "3") ver_grupo();
        else if (op == "4") modificar_grupo();
        else if (op == "5") desactivar_grupo();
        else if (op == "6") reactivar_grupo();
        else if (op == "7") eliminar_grupo();
        else if (op == "8") agregar_profesor_a_grupo();
        else if (op == "0") return;
        else cout << "  [!] Opcion invalida." << endl;
    }
}

void menu_investigadores() {
    while (true) {
        cout << "\n--- MENU INVESTIGADORES ---" << endl;
        cout << "  1. Incluir investigador" << endl;
        cout << "  2. Listar investigadores" << endl;
        cout << "  3. Ver detalle" << endl;
        cout << "  4. Modificar" << endl;
        cout << "  5. Desactivar" << endl;
        cout << "  6. Reactivar" << endl;
        cout << "  7. Eliminar" << endl;
        cout << "  0. Volver" << endl;
        cout << " Opcion: ";
        string op;
        getline(cin, op);

        if      (op == "1") incluir_investigador();
        else if (op == "2") listar_investigadores();
        else if (op == "3") ver_investigador();
        else if (op == "4") modificar_investigador();
        else if (op == "5") desactivar_investigador();
        else if (op == "6") reactivar_investigador();
        else if (op == "7") eliminar_investigador();
        else if (op == "0") return;
        else cout << "  [!] Opcion invalida." << endl;
    }
}

void menu_productos() {
    while (true) {
        cout << "\n--- MENU PRODUCTOS ---" << endl;
        cout << "  1. Incluir producto" << endl;
        cout << "  2. Listar productos" << endl;
        cout << "  3. Ver detalle" << endl;
        cout << "  4. Modificar" << endl;
        cout << "  5. Desactivar" << endl;
        cout << "  6. Reactivar" << endl;
        cout << "  7. Eliminar" << endl;
        cout << "  8. Filtrar por año" << endl;
        cout << "  0. Volver" << endl;
        cout << " Opcion: ";
        string op;
        getline(cin, op);

        if      (op == "1") incluir_producto();
        else if (op == "2") listar_productos();
        else if (op == "3") ver_producto();
        else if (op == "4") modificar_producto();
        else if (op == "5") desactivar_producto();
        else if (op == "6") reactivar_producto();
        else if (op == "7") eliminar_producto();
        else if (op == "8") filtrar_productos_por_anio();
        else if (op == "0") return;
        else cout << "  [!] Opcion invalida." << endl;
    }
}

void menu_principal() {
    cargar_datos();
    cout << "\nDatos cargados: " << grupos.size() << " grupos, "
         << investigadores.size() << " investigadores, "
         << productos.size() << " productos." << endl;

    while (true) {
        cout << "\n==================================================" << endl;
        cout << "   PEA - UPC | Menu principal (C++)" << endl;
        cout << "==================================================" << endl;
        cout << "  1. Gestionar grupos" << endl;
        cout << "  2. Gestionar investigadores" << endl;
        cout << "  3. Gestionar productos" << endl;
        cout << "  4. Importar productos desde CSV" << endl;
        cout << "  5. Resumen estadistico (esquema descriptivo)" << endl;
        cout << "  6. Guardar datos" << endl;
        cout << "  0. Salir" << endl;
        cout << " Opcion: ";
        string op;
        getline(cin, op);

        if (op == "1") {
            menu_grupos();
            guardar_datos();
        }
        else if (op == "2") {
            menu_investigadores();
            guardar_datos();
        }
        else if (op == "3") {
            menu_productos();
            guardar_datos();
        }
        else if (op == "4") {
            importar_csv();
            guardar_datos();
        }
        else if (op == "5") {
            resumen_estadistico();
        }
        else if (op == "6") {
            guardar_datos();
        }
        else if (op == "0") {
            guardar_datos();
            cout << "\n¡Hasta luego!" << endl;
            break;
        }
        else {
            cout << "  [!] Opcion invalida." << endl;
        }
    }
}


// =============================================================================
// PUNTO DE INICIO
// =============================================================================
int main() {
    menu_principal();
    return 0;
}