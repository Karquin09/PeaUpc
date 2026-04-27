"""
===============================================================================
 PEA-UPC: Programa Estadístico de Análisis
 Universidad Popular del Cesar - Estructura de Datos
 Autor: [Tu nombre aquí]
===============================================================================
 Este programa gestiona la información de investigación (grupos,
 investigadores y productos) descargada desde SCIENTI de MinCiencias.

 TODA la información se almacena en LISTAS de Python.
 Cada elemento de la lista es un diccionario que representa un registro.

 ESTRUCTURA DE LOS DATOS:
   grupos         = lista de grupos de investigación
   investigadores = lista de investigadores (profesores)
   productos      = lista de productos (artículos, libros, tesis, etc.)

 VARIABLES DE ENTRADA / SALIDA:
   Grupo:         codigo, nombre, lider, categoria, area, institucion,
                  anio_formacion  -->  estado, productos, integrantes
   Investigador:  cvlac, nombre, categoria, formacion, grupo_codigo
                  -->  estado, productos
   Producto:      id, tipo, titulo, anio, autores, categoria,
                  grupo_codigo, investigador_cvlac  -->  estado, validado
===============================================================================
"""

import csv
import json
import os
from datetime import datetime


# =============================================================================
# LISTAS PRINCIPALES (toda la información vive aquí)
# =============================================================================
grupos = []
investigadores = []
productos = []


# =============================================================================
# CONSTANTES: opciones válidas según MinCiencias
# =============================================================================
CATEGORIAS_GRUPO = ["A1", "A", "B", "C", "Reconocido", "Registrado"]
CATEGORIAS_INVESTIGADOR = ["Emérito", "Senior", "Asociado", "Junior"]
CATEGORIAS_PRODUCTO = ["A1", "A", "B", "C", "D", "Sin categoría"]
TIPOS_PRODUCTO = [
    "Artículo de investigación",
    "Libro resultado de investigación",
    "Capítulo de libro",
    "Software",
    "Tesis de maestría dirigida",
    "Tesis de doctorado dirigida",
    "Ponencia",
    "Producto tecnológico",
]


# =============================================================================
# FUNCIONES AUXILIARES: validar la entrada del usuario
# =============================================================================
def pedir_texto(mensaje):
    """Pide un texto no vacío."""
    while True:
        texto = input(mensaje).strip()
        if texto != "":
            return texto
        print("  [!] No puede estar vacío.")


def pedir_entero(mensaje, minimo, maximo):
    """Pide un número entero dentro de un rango."""
    while True:
        try:
            valor = int(input(mensaje))
            if minimo <= valor <= maximo:
                return valor
            print(f"  [!] Debe estar entre {minimo} y {maximo}.")
        except ValueError:
            print("  [!] Debes ingresar un número entero.")


def pedir_si_no(mensaje):
    """Pide s/n y devuelve True/False."""
    while True:
        r = input(mensaje + " (s/n): ").strip().lower()
        if r == "s":
            return True
        if r == "n":
            return False
        print("  [!] Responde 's' o 'n'.")


def pedir_opcion(mensaje, opciones):
    """Muestra un mini-menú numerado y devuelve la opción elegida."""
    print(mensaje)
    for i in range(len(opciones)):
        print(f"   {i + 1}. {opciones[i]}")
    num = pedir_entero("Elige el número: ", 1, len(opciones))
    return opciones[num - 1]


# =============================================================================
# FUNCIONES DE BÚSQUEDA
# =============================================================================
def buscar_grupo(codigo):
    """Devuelve la posición del grupo o -1 si no existe."""
    for i in range(len(grupos)):
        if grupos[i]["codigo"] == codigo:
            return i
    return -1


def buscar_investigador(cvlac):
    """Devuelve la posición del investigador o -1 si no existe."""
    for i in range(len(investigadores)):
        if investigadores[i]["cvlac"] == cvlac:
            return i
    return -1


def buscar_producto(id_prod):
    """Devuelve la posición del producto o -1 si no existe."""
    for i in range(len(productos)):
        if productos[i]["id"] == id_prod:
            return i
    return -1


def siguiente_id_producto():
    """Genera el próximo ID automático tipo P0001, P0002..."""
    if len(productos) == 0:
        return "P0001"
    # Busca el número más alto usado
    mayor = 0
    for p in productos:
        try:
            numero = int(p["id"][1:])
            if numero > mayor:
                mayor = numero
        except ValueError:
            pass
    return "P" + str(mayor + 1).zfill(4)


# =============================================================================
# GESTIÓN DE GRUPOS
# =============================================================================
def incluir_grupo():
    print("\n>> Nuevo grupo de investigación")
    codigo = pedir_texto("Código del grupo (ej: COL0001): ")
    if buscar_grupo(codigo) != -1:
        print("  [!] Ya existe un grupo con ese código.")
        return

    nombre = pedir_texto("Nombre del grupo: ")
    lider = pedir_texto("Nombre del líder: ")
    categoria = pedir_opcion("Categoría:", CATEGORIAS_GRUPO)
    area = pedir_texto("Área de conocimiento: ")
    institucion = pedir_texto("Institución: ")
    anio = pedir_entero("Año de formación: ", 1900, 2100)

    grupo = {
        "codigo": codigo,
        "nombre": nombre,
        "lider": lider,
        "categoria": categoria,
        "area": area,
        "institucion": institucion,
        "anio_formacion": anio,
        "estado": "activo",
        "productos": [],       # lista de IDs de productos
        "integrantes": [],     # lista de CvLAC de investigadores
    }
    grupos.append(grupo)
    print(f"  [OK] Grupo '{nombre}' creado.")


def listar_grupos():
    print("\n--- GRUPOS ---")
    if len(grupos) == 0:
        print("  (sin grupos)")
        return
    print(f"{'Código':<12} {'Nombre':<30} {'Cat':<5} {'Estado':<10}")
    print("-" * 60)
    for g in grupos:
        print(f"{g['codigo']:<12} {g['nombre'][:30]:<30} "
              f"{g['categoria']:<5} {g['estado']:<10}")


def ver_grupo():
    codigo = pedir_texto("Código del grupo: ")
    i = buscar_grupo(codigo)
    if i == -1:
        print("  [!] No existe ese grupo.")
        return

    g = grupos[i]
    print(f"\n--- DETALLE DEL GRUPO {g['codigo']} ---")
    print(f"  Nombre:       {g['nombre']}")
    print(f"  Líder:        {g['lider']}")
    print(f"  Categoría:    {g['categoria']}")
    print(f"  Área:         {g['area']}")
    print(f"  Institución:  {g['institucion']}")
    print(f"  Año:          {g['anio_formacion']}")
    print(f"  Estado:       {g['estado']}")

    # Mostrar sus profesores (integrantes)
    print(f"\n  Profesores ({len(g['integrantes'])}):")
    for cvlac in g["integrantes"]:
        j = buscar_investigador(cvlac)
        if j != -1:
            print(f"    - {cvlac}: {investigadores[j]['nombre']}")

    # Mostrar sus productos
    print(f"\n  Productos ({len(g['productos'])}):")
    for id_p in g["productos"]:
        k = buscar_producto(id_p)
        if k != -1:
            p = productos[k]
            print(f"    - {p['id']}: {p['titulo'][:50]} ({p['anio']})")


def modificar_grupo():
    codigo = pedir_texto("Código del grupo a modificar: ")
    i = buscar_grupo(codigo)
    if i == -1:
        print("  [!] No existe ese grupo.")
        return

    g = grupos[i]
    print("\n¿Qué quieres modificar?")
    print("   1. Nombre")
    print("   2. Líder")
    print("   3. Categoría")
    print("   4. Área")
    print("   5. Institución")
    print("   6. Año de formación")
    op = pedir_entero("Opción: ", 1, 6)

    if op == 1:
        g["nombre"] = pedir_texto("Nuevo nombre: ")
    elif op == 2:
        g["lider"] = pedir_texto("Nuevo líder: ")
    elif op == 3:
        g["categoria"] = pedir_opcion("Nueva categoría:", CATEGORIAS_GRUPO)
    elif op == 4:
        g["area"] = pedir_texto("Nueva área: ")
    elif op == 5:
        g["institucion"] = pedir_texto("Nueva institución: ")
    elif op == 6:
        g["anio_formacion"] = pedir_entero("Nuevo año: ", 1900, 2100)
    print("  [OK] Grupo modificado.")


def desactivar_grupo():
    codigo = pedir_texto("Código del grupo: ")
    i = buscar_grupo(codigo)
    if i == -1:
        print("  [!] No existe ese grupo.")
        return
    grupos[i]["estado"] = "inactivo"
    print("  [OK] Grupo desactivado.")


def reactivar_grupo():
    codigo = pedir_texto("Código del grupo: ")
    i = buscar_grupo(codigo)
    if i == -1:
        print("  [!] No existe ese grupo.")
        return
    grupos[i]["estado"] = "activo"
    print("  [OK] Grupo reactivado.")


def eliminar_grupo():
    codigo = pedir_texto("Código del grupo a eliminar: ")
    i = buscar_grupo(codigo)
    if i == -1:
        print("  [!] No existe ese grupo.")
        return
    if not pedir_si_no(f"¿Seguro que quieres eliminar '{grupos[i]['nombre']}'?"):
        print("  Cancelado.")
        return

    # Limpiar referencias en investigadores y productos
    for inv in investigadores:
        if inv["grupo_codigo"] == codigo:
            inv["grupo_codigo"] = ""
    for p in productos:
        if p["grupo_codigo"] == codigo:
            p["grupo_codigo"] = ""

    grupos.pop(i)
    print("  [OK] Grupo eliminado.")


def agregar_profesor_a_grupo():
    codigo = pedir_texto("Código del grupo: ")
    i = buscar_grupo(codigo)
    if i == -1:
        print("  [!] Grupo no existe.")
        return
    cvlac = pedir_texto("CvLAC del profesor: ")
    j = buscar_investigador(cvlac)
    if j == -1:
        print("  [!] Investigador no existe.")
        return

    if cvlac not in grupos[i]["integrantes"]:
        grupos[i]["integrantes"].append(cvlac)
    investigadores[j]["grupo_codigo"] = codigo
    print("  [OK] Profesor agregado al grupo.")


# =============================================================================
# GESTIÓN DE INVESTIGADORES
# =============================================================================
def incluir_investigador():
    print("\n>> Nuevo investigador")
    cvlac = pedir_texto("Código CvLAC: ")
    if buscar_investigador(cvlac) != -1:
        print("  [!] Ya existe ese investigador.")
        return

    nombre = pedir_texto("Nombre completo: ")
    categoria = pedir_opcion("Categoría:", CATEGORIAS_INVESTIGADOR)
    formacion = pedir_opcion(
        "Formación:",
        ["Doctorado", "Maestría", "Especialización", "Pregrado"]
    )

    grupo_codigo = ""
    if pedir_si_no("¿Asignar a un grupo ahora?"):
        grupo_codigo = pedir_texto("Código del grupo: ")
        i = buscar_grupo(grupo_codigo)
        if i == -1:
            print("  [!] Grupo no existe. Se crea sin grupo.")
            grupo_codigo = ""
        else:
            if cvlac not in grupos[i]["integrantes"]:
                grupos[i]["integrantes"].append(cvlac)

    investigador = {
        "cvlac": cvlac,
        "nombre": nombre,
        "categoria": categoria,
        "formacion": formacion,
        "grupo_codigo": grupo_codigo,
        "estado": "activo",
        "productos": [],
    }
    investigadores.append(investigador)
    print(f"  [OK] Investigador '{nombre}' creado.")


def listar_investigadores():
    print("\n--- INVESTIGADORES ---")
    if len(investigadores) == 0:
        print("  (sin investigadores)")
        return
    print(f"{'CvLAC':<12} {'Nombre':<25} {'Cat':<10} {'Grupo':<12} {'Estado':<10}")
    print("-" * 70)
    for inv in investigadores:
        print(f"{inv['cvlac']:<12} {inv['nombre'][:25]:<25} "
              f"{inv['categoria']:<10} {inv['grupo_codigo']:<12} "
              f"{inv['estado']:<10}")


def ver_investigador():
    cvlac = pedir_texto("CvLAC: ")
    i = buscar_investigador(cvlac)
    if i == -1:
        print("  [!] No existe ese investigador.")
        return

    inv = investigadores[i]
    print(f"\n--- DETALLE DEL INVESTIGADOR ---")
    print(f"  CvLAC:      {inv['cvlac']}")
    print(f"  Nombre:     {inv['nombre']}")
    print(f"  Categoría:  {inv['categoria']}")
    print(f"  Formación:  {inv['formacion']}")
    print(f"  Grupo:      {inv['grupo_codigo']}")
    print(f"  Estado:     {inv['estado']}")

    print(f"\n  Productos ({len(inv['productos'])}):")
    for id_p in inv["productos"]:
        k = buscar_producto(id_p)
        if k != -1:
            p = productos[k]
            print(f"    - {p['id']}: {p['titulo'][:50]} ({p['anio']})")


def modificar_investigador():
    cvlac = pedir_texto("CvLAC: ")
    i = buscar_investigador(cvlac)
    if i == -1:
        print("  [!] No existe.")
        return

    inv = investigadores[i]
    print("\n¿Qué quieres modificar?")
    print("   1. Nombre")
    print("   2. Categoría")
    print("   3. Formación")
    print("   4. Grupo")
    op = pedir_entero("Opción: ", 1, 4)

    if op == 1:
        inv["nombre"] = pedir_texto("Nuevo nombre: ")
    elif op == 2:
        inv["categoria"] = pedir_opcion("Categoría:", CATEGORIAS_INVESTIGADOR)
    elif op == 3:
        inv["formacion"] = pedir_opcion(
            "Formación:",
            ["Doctorado", "Maestría", "Especialización", "Pregrado"]
        )
    elif op == 4:
        inv["grupo_codigo"] = pedir_texto("Código del grupo: ")
    print("  [OK] Investigador modificado.")


def desactivar_investigador():
    cvlac = pedir_texto("CvLAC: ")
    i = buscar_investigador(cvlac)
    if i == -1:
        print("  [!] No existe.")
        return
    investigadores[i]["estado"] = "inactivo"
    print("  [OK] Desactivado.")


def reactivar_investigador():
    cvlac = pedir_texto("CvLAC: ")
    i = buscar_investigador(cvlac)
    if i == -1:
        print("  [!] No existe.")
        return
    investigadores[i]["estado"] = "activo"
    print("  [OK] Reactivado.")


def eliminar_investigador():
    cvlac = pedir_texto("CvLAC a eliminar: ")
    i = buscar_investigador(cvlac)
    if i == -1:
        print("  [!] No existe.")
        return
    if not pedir_si_no(f"¿Eliminar '{investigadores[i]['nombre']}'?"):
        print("  Cancelado.")
        return

    # Quitar de los grupos
    for g in grupos:
        if cvlac in g["integrantes"]:
            g["integrantes"].remove(cvlac)
    # Quitar de los productos
    for p in productos:
        if p["investigador_cvlac"] == cvlac:
            p["investigador_cvlac"] = ""

    investigadores.pop(i)
    print("  [OK] Eliminado.")


# =============================================================================
# GESTIÓN DE PRODUCTOS
# =============================================================================
def incluir_producto():
    print("\n>> Nuevo producto de investigación")
    id_prod = siguiente_id_producto()
    tipo = pedir_opcion("Tipo de producto:", TIPOS_PRODUCTO)
    titulo = pedir_texto("Título: ")
    anio = pedir_entero("Año: ", 1900, 2100)

    autores_txt = pedir_texto("Autores (separados por coma): ")
    autores = []
    for a in autores_txt.split(","):
        a = a.strip()
        if a != "":
            autores.append(a)

    categoria = pedir_opcion("Categoría MinCiencias:", CATEGORIAS_PRODUCTO)

    grupo_codigo = ""
    if pedir_si_no("¿Asociar a un grupo?"):
        grupo_codigo = pedir_texto("Código del grupo: ")
        if buscar_grupo(grupo_codigo) == -1:
            print("  [!] Grupo no existe. Se crea sin grupo.")
            grupo_codigo = ""

    investigador_cvlac = ""
    if pedir_si_no("¿Asociar a un investigador?"):
        investigador_cvlac = pedir_texto("CvLAC: ")
        if buscar_investigador(investigador_cvlac) == -1:
            print("  [!] Investigador no existe. Se crea sin investigador.")
            investigador_cvlac = ""

    validado = pedir_si_no("¿Producto validado por el comité?")

    producto = {
        "id": id_prod,
        "tipo": tipo,
        "titulo": titulo,
        "anio": anio,
        "autores": autores,
        "categoria": categoria,
        "grupo_codigo": grupo_codigo,
        "investigador_cvlac": investigador_cvlac,
        "validado": validado,
        "estado": "activo",
    }
    productos.append(producto)

    # Mantener las relaciones bidireccionales
    if grupo_codigo != "":
        i = buscar_grupo(grupo_codigo)
        if i != -1 and id_prod not in grupos[i]["productos"]:
            grupos[i]["productos"].append(id_prod)
    if investigador_cvlac != "":
        j = buscar_investigador(investigador_cvlac)
        if j != -1 and id_prod not in investigadores[j]["productos"]:
            investigadores[j]["productos"].append(id_prod)

    print(f"  [OK] Producto '{titulo}' registrado con ID {id_prod}.")


def listar_productos():
    print("\n--- PRODUCTOS ---")
    if len(productos) == 0:
        print("  (sin productos)")
        return
    print(f"{'ID':<7} {'Título':<40} {'Año':<6} {'Cat':<12} {'Val':<5} {'Estado':<10}")
    print("-" * 85)
    for p in productos:
        val = "Sí" if p["validado"] else "No"
        print(f"{p['id']:<7} {p['titulo'][:40]:<40} {p['anio']:<6} "
              f"{p['categoria']:<12} {val:<5} {p['estado']:<10}")


def ver_producto():
    id_prod = pedir_texto("ID del producto: ")
    i = buscar_producto(id_prod)
    if i == -1:
        print("  [!] No existe.")
        return
    p = productos[i]
    print(f"\n--- DETALLE DEL PRODUCTO {p['id']} ---")
    print(f"  Tipo:        {p['tipo']}")
    print(f"  Título:      {p['titulo']}")
    print(f"  Año:         {p['anio']}")
    print(f"  Autores:     {', '.join(p['autores'])}")
    print(f"  Categoría:   {p['categoria']}")
    print(f"  Grupo:       {p['grupo_codigo']}")
    print(f"  Invest.:     {p['investigador_cvlac']}")
    print(f"  Validado:    {'Sí' if p['validado'] else 'No'}")
    print(f"  Estado:      {p['estado']}")


def modificar_producto():
    id_prod = pedir_texto("ID del producto: ")
    i = buscar_producto(id_prod)
    if i == -1:
        print("  [!] No existe.")
        return

    p = productos[i]
    print("\n¿Qué quieres modificar?")
    print("   1. Tipo")
    print("   2. Título")
    print("   3. Año")
    print("   4. Autores")
    print("   5. Categoría")
    print("   6. Validado (sí/no)")
    op = pedir_entero("Opción: ", 1, 6)

    if op == 1:
        p["tipo"] = pedir_opcion("Tipo:", TIPOS_PRODUCTO)
    elif op == 2:
        p["titulo"] = pedir_texto("Nuevo título: ")
    elif op == 3:
        p["anio"] = pedir_entero("Nuevo año: ", 1900, 2100)
    elif op == 4:
        texto = pedir_texto("Autores (separados por coma): ")
        p["autores"] = [a.strip() for a in texto.split(",") if a.strip()]
    elif op == 5:
        p["categoria"] = pedir_opcion("Categoría:", CATEGORIAS_PRODUCTO)
    elif op == 6:
        p["validado"] = pedir_si_no("¿Validado?")
    print("  [OK] Producto modificado.")


def desactivar_producto():
    id_prod = pedir_texto("ID del producto: ")
    i = buscar_producto(id_prod)
    if i == -1:
        print("  [!] No existe.")
        return
    productos[i]["estado"] = "inactivo"
    print("  [OK] Desactivado.")


def reactivar_producto():
    id_prod = pedir_texto("ID del producto: ")
    i = buscar_producto(id_prod)
    if i == -1:
        print("  [!] No existe.")
        return
    productos[i]["estado"] = "activo"
    print("  [OK] Reactivado.")


def eliminar_producto():
    id_prod = pedir_texto("ID del producto: ")
    i = buscar_producto(id_prod)
    if i == -1:
        print("  [!] No existe.")
        return
    if not pedir_si_no(f"¿Eliminar '{productos[i]['titulo']}'?"):
        print("  Cancelado.")
        return

    # Quitar referencias
    for g in grupos:
        if id_prod in g["productos"]:
            g["productos"].remove(id_prod)
    for inv in investigadores:
        if id_prod in inv["productos"]:
            inv["productos"].remove(id_prod)

    productos.pop(i)
    print("  [OK] Eliminado.")


# =============================================================================
# FILTRAR PRODUCTOS POR AÑO
# =============================================================================
def filtrar_productos_por_anio():
    print("\n>> Filtrar productos por año")
    print("   1. Últimos 2 años")
    print("   2. Últimos 5 años")
    print("   3. Últimos 10 años")
    print("   4. Rango personalizado")
    op = pedir_entero("Opción: ", 1, 4)

    anio_actual = datetime.now().year

    if op == 1:
        desde = anio_actual - 1
        hasta = anio_actual
    elif op == 2:
        desde = anio_actual - 4
        hasta = anio_actual
    elif op == 3:
        desde = anio_actual - 9
        hasta = anio_actual
    else:
        desde = pedir_entero("Año desde: ", 1900, anio_actual)
        hasta = pedir_entero("Año hasta: ", desde, anio_actual)

    # Filtrar
    resultado = []
    for p in productos:
        if desde <= p["anio"] <= hasta:
            resultado.append(p)

    print(f"\n--- Productos entre {desde} y {hasta}: {len(resultado)} ---")
    for p in resultado:
        print(f"  {p['id']}: {p['titulo'][:50]} ({p['anio']}) cat={p['categoria']}")


# =============================================================================
# PERSISTENCIA: guardar y cargar
# =============================================================================
def guardar_datos():
    """Guarda las 3 listas en archivos JSON dentro de la carpeta 'datos/'."""
    if not os.path.exists("datos"):
        os.makedirs("datos")

    with open("datos/grupos.json", "w", encoding="utf-8") as f:
        json.dump(grupos, f, ensure_ascii=False, indent=2)

    with open("datos/investigadores.json", "w", encoding="utf-8") as f:
        json.dump(investigadores, f, ensure_ascii=False, indent=2)

    with open("datos/productos.json", "w", encoding="utf-8") as f:
        json.dump(productos, f, ensure_ascii=False, indent=2)

    print("  [OK] Datos guardados en la carpeta 'datos/'.")


def cargar_datos():
    """Carga las 3 listas desde los archivos JSON si existen."""
    global grupos, investigadores, productos

    if os.path.exists("datos/grupos.json"):
        with open("datos/grupos.json", "r", encoding="utf-8") as f:
            grupos = json.load(f)

    if os.path.exists("datos/investigadores.json"):
        with open("datos/investigadores.json", "r", encoding="utf-8") as f:
            investigadores = json.load(f)

    if os.path.exists("datos/productos.json"):
        with open("datos/productos.json", "r", encoding="utf-8") as f:
            productos = json.load(f)


# =============================================================================
# IMPORTAR DATOS (desde CSV, URL de SCIENTI, o PDF)
# =============================================================================
def importar_csv():
    """Importa productos desde un archivo CSV con separador ';'."""
    ruta = pedir_texto("Ruta del archivo CSV: ")
    # Limpiar comillas por si Windows las copió al pegar la ruta
    ruta = ruta.strip('"').strip("'")
    if not os.path.exists(ruta):
        print("  [!] No se encontró el archivo.")
        print(f"      Busqué en: {ruta}")
        return

    añadidos = 0
    with open(ruta, "r", encoding="utf-8") as f:
        lector = csv.DictReader(f, delimiter=";")
        for fila in lector:
            # Si ya existe, lo saltamos
            if buscar_producto(fila.get("id", "")) != -1:
                continue

            autores = []
            if fila.get("autores", ""):
                for a in fila["autores"].split("|"):
                    if a.strip():
                        autores.append(a.strip())

            try:
                anio = int(fila.get("anio", 0))
            except ValueError:
                anio = 0

            validado = fila.get("validado", "").lower() in ("true", "1", "si", "sí")

            producto = {
                "id": fila.get("id", siguiente_id_producto()),
                "tipo": fila.get("tipo", "Artículo de investigación"),
                "titulo": fila.get("titulo", "(sin título)"),
                "anio": anio,
                "autores": autores,
                "categoria": fila.get("categoria", "Sin categoría"),
                "grupo_codigo": fila.get("grupo_codigo", ""),
                "investigador_cvlac": fila.get("investigador_cvlac", ""),
                "validado": validado,
                "estado": "activo",
            }
            productos.append(producto)

            # Mantener relaciones
            if producto["grupo_codigo"]:
                i = buscar_grupo(producto["grupo_codigo"])
                if i != -1 and producto["id"] not in grupos[i]["productos"]:
                    grupos[i]["productos"].append(producto["id"])
            if producto["investigador_cvlac"]:
                j = buscar_investigador(producto["investigador_cvlac"])
                if j != -1 and producto["id"] not in investigadores[j]["productos"]:
                    investigadores[j]["productos"].append(producto["id"])

            añadidos += 1

    print(f"  [OK] Productos importados: {añadidos}")


def importar_desde_url():
    """Descarga una página de SCIENTI (GrupLAC o CvLAC) y extrae datos.
    Usa 'requests' si está disponible, o la librería estándar 'urllib'."""
    try:
        import requests
    except ImportError:
        requests = None

    url = pedir_texto("URL de SCIENTI (GrupLAC o CvLAC): ")
    print(f"  ... descargando {url}")

    try:
        # Algunas páginas requieren un User-Agent para responder
        headers = {"User-Agent": "Mozilla/5.0"}
        respuesta = requests.get(url, headers=headers, timeout=20)
    except Exception as error:
        print(f"  [!] Error de conexión: {error}")
        return

    if respuesta.status_code != 200:
        print(f"  [!] El servidor respondió con código {respuesta.status_code}")
        return

    html = respuesta.text
    encontrados = procesar_html_scienti(html, url)
    print(f"  [OK] Datos extraídos de la URL. Productos detectados: {encontrados}")


def procesar_html_scienti(html, origen):
    """Procesa el HTML de SCIENTI con expresiones regulares.
    Detecta si es GrupLAC (grupo) o CvLAC (investigador) y busca productos."""
    import re

    # Quitar etiquetas HTML para tener texto plano
    texto = re.sub(r"<[^>]+>", " ", html)
    texto = re.sub(r"\s+", " ", texto)

    encontrados = 0

    # ¿Es página de grupo (GrupLAC) o de investigador (CvLAC)?
    es_gruplac = "gruplac" in origen.lower() or "Grupo de Investigación" in texto
    es_cvlac = "cvlac" in origen.lower() or "cod_rh=" in origen

    grupo_codigo = ""
    investigador_cvlac = ""

    if es_gruplac:
        # Buscar el código del grupo (formato COL seguido de números)
        match = re.search(r"COL\d{7,}", texto)
        if match:
            grupo_codigo = match.group(0)
            if buscar_grupo(grupo_codigo) == -1:
                grupos.append({
                    "codigo": grupo_codigo,
                    "nombre": "Grupo importado de SCIENTI",
                    "lider": "(por definir)",
                    "categoria": "Reconocido",
                    "area": "(por definir)",
                    "institucion": "Universidad Popular del Cesar",
                    "anio_formacion": 2000,
                    "estado": "activo",
                    "productos": [],
                    "integrantes": [],
                })
                print(f"  [i] Se creó un grupo nuevo con código {grupo_codigo}")

    if es_cvlac:
        # Buscar el código CvLAC
        match = re.search(r"cod_rh=(\d+)", origen)
        if match:
            investigador_cvlac = match.group(1)
            if buscar_investigador(investigador_cvlac) == -1:
                investigadores.append({
                    "cvlac": investigador_cvlac,
                    "nombre": "Investigador importado de SCIENTI",
                    "categoria": "Junior",
                    "formacion": "Pregrado",
                    "grupo_codigo": "",
                    "estado": "activo",
                    "productos": [],
                })
                print(f"  [i] Se creó un investigador con CvLAC {investigador_cvlac}")

    # Buscar productos: cualquier frase larga que contenga un año entre 1990 y 2026
    # es probablemente un producto de investigación
    patron_anio = re.compile(r".{20,200}\b(19[9]\d|20[0-2]\d)\b.{0,100}")
    for fragmento in patron_anio.findall(texto)[:50]:  # máximo 50 productos
        # findall con grupo devuelve solo el año, así que volvemos a buscar la frase
        pass

    # Mejor enfoque: dividir el texto en líneas y buscar las que tengan año
    for trozo in re.split(r"[.|]", texto):
        trozo = trozo.strip()
        if len(trozo) < 30 or len(trozo) > 250:
            continue
        match_anio = re.search(r"\b(19\d{2}|20[0-2]\d)\b", trozo)
        if not match_anio:
            continue

        anio = int(match_anio.group(0))
        titulo = trozo[:150]

        # Evitar duplicados
        ya_existe = False
        for p in productos:
            if p["titulo"] == titulo:
                ya_existe = True
                break
        if ya_existe:
            continue

        id_prod = siguiente_id_producto()
        producto = {
            "id": id_prod,
            "tipo": "Artículo de investigación",
            "titulo": titulo,
            "anio": anio,
            "autores": [],
            "categoria": "Sin categoría",
            "grupo_codigo": grupo_codigo,
            "investigador_cvlac": investigador_cvlac,
            "validado": False,
            "estado": "activo",
        }
        productos.append(producto)

        # Mantener relaciones
        if grupo_codigo:
            i = buscar_grupo(grupo_codigo)
            if i != -1:
                grupos[i]["productos"].append(id_prod)
        if investigador_cvlac:
            j = buscar_investigador(investigador_cvlac)
            if j != -1:
                investigadores[j]["productos"].append(id_prod)

        encontrados += 1
        if encontrados >= 50:  # límite de seguridad
            break

    return encontrados


def importar_desde_pdf():
    """Lee un archivo PDF y extrae productos.
    Necesita: pip install pdfplumber"""
    try:
        import pdfplumber
    except ImportError:
        print("  [!] Falta la librería 'pdfplumber'. Ejecuta:")
        print("      pip install pdfplumber")
        return

    ruta = pedir_texto("Ruta del archivo PDF: ")
    ruta = ruta.strip('"').strip("'")
    if not os.path.exists(ruta):
        print("  [!] No se encontró el archivo.")
        return

    print(f"  ... leyendo {ruta}")
    texto_total = ""
    try:
        with pdfplumber.open(ruta) as pdf:
            for pagina in pdf.pages:
                texto_pagina = pagina.extract_text()
                if texto_pagina:
                    texto_total += "\n" + texto_pagina
    except Exception as error:
        print(f"  [!] Error al leer el PDF: {error}")
        return

    # Buscar líneas que contengan un año (probables productos)
    import re
    encontrados = 0
    for linea in texto_total.split("\n"):
        linea = linea.strip()
        if len(linea) < 30 or len(linea) > 250:
            continue
        match_anio = re.search(r"\b(19\d{2}|20[0-2]\d)\b", linea)
        if not match_anio:
            continue

        anio = int(match_anio.group(0))
        titulo = linea[:150]

        # Evitar duplicados
        ya_existe = False
        for p in productos:
            if p["titulo"] == titulo:
                ya_existe = True
                break
        if ya_existe:
            continue

        id_prod = siguiente_id_producto()
        productos.append({
            "id": id_prod,
            "tipo": "Artículo de investigación",
            "titulo": titulo,
            "anio": anio,
            "autores": [],
            "categoria": "Sin categoría",
            "grupo_codigo": "",
            "investigador_cvlac": "",
            "validado": False,
            "estado": "activo",
        })
        encontrados += 1
        if encontrados >= 100:  # límite de seguridad
            break

    print(f"  [OK] PDF procesado. Productos detectados: {encontrados}")


def menu_importar():
    """Menú con las 3 opciones de importación que pide el enunciado."""
    while True:
        print("\n--- IMPORTAR DATOS ---")
        print("  1. Desde archivo CSV")
        print("  2. Desde URL de SCIENTI (GrupLAC / CvLAC)")
        print("  3. Desde archivo PDF")
        print("  0. Volver")
        op = input(" Opción: ").strip()

        if op == "1":
            importar_csv()
        elif op == "2":
            importar_desde_url()
        elif op == "3":
            importar_desde_pdf()
        elif op == "0":
            return
        else:
            print("  [!] Opción inválida.")


# =============================================================================
# DASHBOARD (estadísticas)
# =============================================================================
def resumen_en_texto():
    """Muestra un resumen de los datos en formato de texto en la consola."""
    print("\n" + "=" * 60)
    print("  RESUMEN ESTADÍSTICO - PEA UPC")
    print("=" * 60)

    # Totales
    activos_g = 0
    for g in grupos:
        if g["estado"] == "activo":
            activos_g += 1
    activos_i = 0
    for inv in investigadores:
        if inv["estado"] == "activo":
            activos_i += 1
    validados = 0
    for p in productos:
        if p["validado"]:
            validados += 1

    print(f"  Grupos:         {len(grupos)} ({activos_g} activos)")
    print(f"  Investigadores: {len(investigadores)} ({activos_i} activos)")
    print(f"  Productos:      {len(productos)} ({validados} validados)")

    # Contar productos por año (últimos 10 años) - usando diccionario manual
    anio_actual = datetime.now().year
    conteo_anios = {}
    for p in productos:
        if p["estado"] == "activo" and anio_actual - 9 <= p["anio"] <= anio_actual:
            if p["anio"] in conteo_anios:
                conteo_anios[p["anio"]] += 1
            else:
                conteo_anios[p["anio"]] = 1

    if len(conteo_anios) > 0:
        print("\n  Productos por año (últimos 10 años):")
        for anio in sorted(conteo_anios.keys()):
            barras = "#" * conteo_anios[anio]
            print(f"    {anio}: {barras} ({conteo_anios[anio]})")

    # Contar productos por categoría
    conteo_cat = {}
    for p in productos:
        if p["estado"] == "activo":
            cat = p["categoria"]
            if cat in conteo_cat:
                conteo_cat[cat] += 1
            else:
                conteo_cat[cat] = 1

    if len(conteo_cat) > 0:
        print("\n  Productos por categoría:")
        for cat in conteo_cat:
            print(f"    {cat}: {conteo_cat[cat]}")

    print("=" * 60)


def dashboard_grafico():
    """Dashboard con gráficos usando matplotlib."""
    try:
        import matplotlib.pyplot as plt
    except ImportError:
        print("  [!] Falta matplotlib. Ejecuta: pip install matplotlib")
        return

    if len(productos) == 0:
        print("  [!] No hay productos para mostrar.")
        return

    # Preparar datos
    anios = []
    categorias = {}
    tipos = {}
    validados = 0
    no_validados = 0

    for p in productos:
        if p["estado"] != "activo":
            continue
        anios.append(p["anio"])
        # Contar categorías
        c = p["categoria"]
        categorias[c] = categorias.get(c, 0) + 1
        # Contar tipos
        t = p["tipo"]
        tipos[t] = tipos.get(t, 0) + 1
        # Contar validación
        if p["validado"]:
            validados += 1
        else:
            no_validados += 1

    # Crear ventana con 4 gráficos (2x2)
    fig, axes = plt.subplots(2, 2, figsize=(12, 8))
    fig.suptitle("PEA-UPC | Dashboard de Investigación", fontsize=14)

    # Gráfico 1: histograma por año
    axes[0, 0].hist(anios, bins=10, color="blue", edgecolor="white")
    axes[0, 0].set_title("Productos por año")
    axes[0, 0].set_xlabel("Año")
    axes[0, 0].set_ylabel("Cantidad")

    # Gráfico 2: barras por categoría
    axes[0, 1].bar(categorias.keys(), categorias.values(), color="green")
    axes[0, 1].set_title("Productos por categoría")
    axes[0, 1].set_ylabel("Cantidad")

    # Gráfico 3: barras por tipo
    nombres_tipos = list(tipos.keys())
    valores_tipos = list(tipos.values())
    axes[1, 0].barh(nombres_tipos, valores_tipos, color="orange")
    axes[1, 0].set_title("Productos por tipo")

    # Gráfico 4: pie de validación
    if validados + no_validados > 0:
        axes[1, 1].pie(
            [validados, no_validados],
            labels=["Validados", "Sin validar"],
            colors=["lightgreen", "lightgray"],
            autopct="%1.1f%%"
        )
        axes[1, 1].set_title("Estado de validación")

    plt.tight_layout()
    plt.show()


# =============================================================================
# MENÚS
# =============================================================================
def menu_grupos():
    while True:
        print("\n--- MENÚ GRUPOS ---")
        print("  1. Incluir grupo")
        print("  2. Listar grupos")
        print("  3. Ver detalle")
        print("  4. Modificar grupo")
        print("  5. Desactivar grupo")
        print("  6. Reactivar grupo")
        print("  7. Eliminar grupo")
        print("  8. Agregar profesor a un grupo")
        print("  0. Volver")
        op = input(" Opción: ").strip()

        if op == "1": incluir_grupo()
        elif op == "2": listar_grupos()
        elif op == "3": ver_grupo()
        elif op == "4": modificar_grupo()
        elif op == "5": desactivar_grupo()
        elif op == "6": reactivar_grupo()
        elif op == "7": eliminar_grupo()
        elif op == "8": agregar_profesor_a_grupo()
        elif op == "0": return
        else: print("  [!] Opción inválida.")


def menu_investigadores():
    while True:
        print("\n--- MENÚ INVESTIGADORES ---")
        print("  1. Incluir investigador")
        print("  2. Listar investigadores")
        print("  3. Ver detalle")
        print("  4. Modificar")
        print("  5. Desactivar")
        print("  6. Reactivar")
        print("  7. Eliminar")
        print("  0. Volver")
        op = input(" Opción: ").strip()

        if op == "1": incluir_investigador()
        elif op == "2": listar_investigadores()
        elif op == "3": ver_investigador()
        elif op == "4": modificar_investigador()
        elif op == "5": desactivar_investigador()
        elif op == "6": reactivar_investigador()
        elif op == "7": eliminar_investigador()
        elif op == "0": return
        else: print("  [!] Opción inválida.")


def menu_productos():
    while True:
        print("\n--- MENÚ PRODUCTOS ---")
        print("  1. Incluir producto")
        print("  2. Listar productos")
        print("  3. Ver detalle")
        print("  4. Modificar")
        print("  5. Desactivar")
        print("  6. Reactivar")
        print("  7. Eliminar")
        print("  8. Filtrar por año")
        print("  0. Volver")
        op = input(" Opción: ").strip()

        if op == "1": incluir_producto()
        elif op == "2": listar_productos()
        elif op == "3": ver_producto()
        elif op == "4": modificar_producto()
        elif op == "5": desactivar_producto()
        elif op == "6": reactivar_producto()
        elif op == "7": eliminar_producto()
        elif op == "8": filtrar_productos_por_anio()
        elif op == "0": return
        else: print("  [!] Opción inválida.")


def menu_principal():
    cargar_datos()
    print(f"\nDatos cargados: {len(grupos)} grupos, "
          f"{len(investigadores)} investigadores, "
          f"{len(productos)} productos.")

    while True:
        print("\n" + "=" * 50)
        print("   PEA - UPC | Menú principal")
        print("=" * 50)
        print("  1. Gestionar grupos")
        print("  2. Gestionar investigadores")
        print("  3. Gestionar productos")
        print("  4. Importar datos (CSV / URL / PDF)")
        print("  5. Resumen estadístico (texto)")
        print("  6. Dashboard con gráficos")
        print("  7. Guardar datos")
        print("  0. Salir")
        op = input(" Opción: ").strip()

        if op == "1":
            menu_grupos()
            guardar_datos()
        elif op == "2":
            menu_investigadores()
            guardar_datos()
        elif op == "3":
            menu_productos()
            guardar_datos()
        elif op == "4":
            menu_importar()
            guardar_datos()
        elif op == "5":
            resumen_en_texto()
        elif op == "6":
            dashboard_grafico()
        elif op == "7":
            guardar_datos()
        elif op == "0":
            guardar_datos()
            print("\n¡Hasta luego!\n")
            break
        else:
            print("  [!] Opción inválida.")


# =============================================================================
# PUNTO DE INICIO DEL PROGRAMA
# =============================================================================
if __name__ == "__main__":
    menu_principal()
