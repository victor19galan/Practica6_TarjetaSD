
#include <Arduino.h>
#include <SPI.h>
#include <SD.h>

#define SD_CS 5   // Cambia a 15 si compartes SPI con RC522

void printLine() {
  Serial.println("--------------------------------");
}

void mostrarInfoSD() {
  uint8_t tipo = SD.cardType();

  Serial.println("INFO TARJETA SD");

  if (tipo == CARD_NONE) {
    Serial.println("No hay tarjeta SD");
    return;
  }

  Serial.print("Tipo: ");
  if (tipo == CARD_MMC) Serial.println("MMC");
  else if (tipo == CARD_SD) Serial.println("SDSC");
  else if (tipo == CARD_SDHC) Serial.println("SDHC");
  else Serial.println("Desconocido");

  Serial.print("Tamaño total: ");
  Serial.print(SD.cardSize() / (1024 * 1024));
  Serial.println(" MB");

  Serial.print("Espacio usado: ");
  Serial.print(SD.usedBytes() / (1024 * 1024));
  Serial.println(" MB");

  Serial.print("Espacio total FS: ");
  Serial.print(SD.totalBytes() / (1024 * 1024));
  Serial.println(" MB");
}

void crearDirectorio(const char *ruta) {
  if (SD.mkdir(ruta)) {
    Serial.print("Directorio creado: ");
    Serial.println(ruta);
  } else {
    Serial.print("Error creando directorio: ");
    Serial.println(ruta);
  }
}

void borrarDirectorio(const char *ruta) {
  if (SD.rmdir(ruta)) {
    Serial.print("Directorio borrado: ");
    Serial.println(ruta);
  } else {
    Serial.print("Error borrando directorio: ");
    Serial.println(ruta);
    Serial.println("Nota: debe estar vacío.");
  }
}

void escribirArchivo(const char *ruta, const char *mensaje) {
  File file = SD.open(ruta, FILE_WRITE);

  if (!file) {
    Serial.print("Error abriendo para escribir: ");
    Serial.println(ruta);
    return;
  }

  file.print(mensaje);
  file.close();

  Serial.print("Archivo escrito: ");
  Serial.println(ruta);
}

void anadirArchivo(const char *ruta, const char *mensaje) {
  File file = SD.open(ruta, FILE_APPEND);

  if (!file) {
    Serial.print("Error abriendo para añadir: ");
    Serial.println(ruta);
    return;
  }

  file.print(mensaje);
  file.close();

  Serial.print("Datos añadidos a: ");
  Serial.println(ruta);
}

void leerArchivo(const char *ruta) {
  File file = SD.open(ruta);

  if (!file) {
    Serial.print("No se pudo abrir: ");
    Serial.println(ruta);
    return;
  }

  Serial.print("Contenido de ");
  Serial.println(ruta);

  while (file.available()) {
    Serial.write(file.read());
  }

  Serial.println();
  file.close();
}

void borrarArchivo(const char *ruta) {
  if (SD.remove(ruta)) {
    Serial.print("Archivo borrado: ");
    Serial.println(ruta);
  } else {
    Serial.print("Error borrando archivo: ");
    Serial.println(ruta);
  }
}

void renombrarArchivo(const char *origen, const char *destino) {
  if (SD.rename(origen, destino)) {
    Serial.print("Renombrado: ");
    Serial.print(origen);
    Serial.print(" -> ");
    Serial.println(destino);
  } else {
    Serial.println("Error renombrando archivo");
  }
}

void comprobarExiste(const char *ruta) {
  if (SD.exists(ruta)) {
    Serial.print("Existe: ");
    Serial.println(ruta);
  } else {
    Serial.print("No existe: ");
    Serial.println(ruta);
  }
}

void mostrarArbol(File dir, int nivel = 0) {
  while (true) {
    File entry = dir.openNextFile();

    if (!entry) break;

    for (int i = 0; i < nivel; i++) {
      Serial.print("  ");
    }

    if (entry.isDirectory()) {
      Serial.print("[DIR] ");
      Serial.println(entry.name());

      mostrarArbol(entry, nivel + 1);
    } else {
      Serial.print("[FILE] ");
      Serial.print(entry.name());
      Serial.print(" - ");
      Serial.print(entry.size());
      Serial.println(" bytes");
    }

    entry.close();
  }
}

uint64_t calcularTamanoDirectorio(File dir) {
  uint64_t total = 0;

  while (true) {
    File entry = dir.openNextFile();

    if (!entry) break;

    if (entry.isDirectory()) {
      total += calcularTamanoDirectorio(entry);
    } else {
      total += entry.size();
    }

    entry.close();
  }

  return total;
}

void mostrarTamanoDirectorio(const char *ruta) {
  File dir = SD.open(ruta);

  if (!dir || !dir.isDirectory()) {
    Serial.print("No es directorio: ");
    Serial.println(ruta);
    return;
  }

  uint64_t tamano = calcularTamanoDirectorio(dir);

  Serial.print("Tamaño de ");
  Serial.print(ruta);
  Serial.print(": ");
  Serial.print(tamano);
  Serial.println(" bytes");

  dir.close();
}

void copiarArchivo(const char *origen, const char *destino) {
  File src = SD.open(origen, FILE_READ);

  if (!src) {
    Serial.print("No se pudo abrir origen: ");
    Serial.println(origen);
    return;
  }

  File dst = SD.open(destino, FILE_WRITE);

  if (!dst) {
    Serial.print("No se pudo abrir destino: ");
    Serial.println(destino);
    src.close();
    return;
  }

  uint8_t buffer[64];

  while (src.available()) {
    size_t bytesLeidos = src.read(buffer, sizeof(buffer));
    dst.write(buffer, bytesLeidos);
  }

  src.close();
  dst.close();

  Serial.print("Copiado: ");
  Serial.print(origen);
  Serial.print(" -> ");
  Serial.println(destino);
}

void borrarDirectorioRecursivo(const char *ruta) {
  File dir = SD.open(ruta);

  if (!dir || !dir.isDirectory()) {
    Serial.print("No es directorio: ");
    Serial.println(ruta);
    return;
  }

  while (true) {
    File entry = dir.openNextFile();

    if (!entry) break;

    String path = String(ruta) + "/" + entry.name();

    if (entry.isDirectory()) {
      entry.close();
      borrarDirectorioRecursivo(path.c_str());
    } else {
      Serial.print("Borrando archivo: ");
      Serial.println(path);
      SD.remove(path.c_str());
      entry.close();
    }
  }

  dir.close();

  if (SD.rmdir(ruta)) {
    Serial.print("Directorio borrado recursivamente: ");
    Serial.println(ruta);
  } else {
    Serial.print("No se pudo borrar directorio: ");
    Serial.println(ruta);
  }
}

void ejecutarDemo() {
  printLine();
  mostrarInfoSD();

  printLine();
  crearDirectorio("/practica");
  crearDirectorio("/practica/logs");
  crearDirectorio("/practica/datos");

  printLine();
  escribirArchivo("/practica/info.txt", "Practica SD con ESP32\n");
  anadirArchivo("/practica/info.txt", "Linea añadida al archivo\n");
  anadirArchivo("/practica/info.txt", "Otra linea de prueba\n");

  escribirArchivo("/practica/logs/log1.txt", "LOG 1: Sistema iniciado\n");
  escribirArchivo("/practica/logs/log2.txt", "LOG 2: Lectura correcta\n");
  escribirArchivo("/practica/datos/sensor.txt", "Temperatura: 24.5\nHumedad: 61\n");

  printLine();
  leerArchivo("/practica/info.txt");

  printLine();
  comprobarExiste("/practica/info.txt");
  comprobarExiste("/practica/noexiste.txt");

  printLine();
  copiarArchivo("/practica/info.txt", "/practica/copia_info.txt");

  printLine();
  renombrarArchivo("/practica/copia_info.txt", "/practica/info_backup.txt");

  printLine();
  Serial.println("Árbol de directorios:");
  File root = SD.open("/");
  mostrarArbol(root);
  root.close();

  printLine();
  mostrarTamanoDirectorio("/practica");

  printLine();
  borrarArchivo("/practica/logs/log2.txt");

  printLine();
  Serial.println("Árbol después de borrar un archivo:");
  root = SD.open("/");
  mostrarArbol(root);
  root.close();

  printLine();
  Serial.println("Para probar borrado recursivo, descomenta esta línea:");
  Serial.println("// borrarDirectorioRecursivo(\"/practica\");");
}

void setup() {
  Serial.begin(115200);
  delay(1000);

  Serial.println();
  Serial.println("Iniciando SD...");

  if (!SD.begin(SD_CS)) {
    Serial.println("ERROR: No se pudo iniciar la tarjeta SD");
    Serial.println("Revisa CS, alimentación, cables y formato FAT32.");
    return;
  }

  Serial.println("SD iniciada correctamente");

  ejecutarDemo();
}

void loop() {
}