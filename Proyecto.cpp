#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <cstdlib>
#include <ctime>

using namespace std;

// Se declararon las funciones
void generarEscenarioAleatorio(const string& nombre_archivo, int filas, int columnas, int prob_inicial);
void cargarEscenario(const string& nombre_archivo, vector<string>& matriz_ciudad);
void imprimirMatriz(const vector<string>& matriz_ciudad);
int mostrarMenu();
void avanzarCiclo(vector<string>& matriz_ciudad, int& dias, int prob_contagio, int prob_recuperacion);
void aplicarIntervencion(vector<string>& matriz_ciudad, int tipo_accion, int fila, int columna);
void guardarReporte(int dias, int total_sanos, int total_infectados, int total_recuperados);
void superVacunaRecursiva(vector<string>& matriz_ciudad, int fila, int columna, int& vacunados, int limite_dosis);

// --- FUNCIÓN PRINCIPAL ---
int main() {
    srand(time(0)); 
    vector<string> ciudad;
    int dias = 0;
    string nombre_archivo = "ciudad.txt";
    
    int filas, columnas, prob_inicial, prob_contagio, prob_recuperacion, limite_vacunas;
    int eleccion_modo;

    cout << "=== INICIANDO SIMULADOR EPIDEMIOLOGICO ===" << endl;
    cout << "\nSELECCIONE EL MODO DE JUEGO:" << endl;
    cout << "1. Modo Rapido    (Ciudad pequena 8x8, virus muy agresivo)" << endl;
    cout << "2. Modo Extendido (Ciudad grande 20x20, propagacion constante)" << endl;
    cout << "Opcion: ";
    cin >> eleccion_modo;

    if (eleccion_modo == 1) {
        filas = 8; 
        columnas = 8;
        prob_inicial = 25;      
        prob_contagio = 30;     
        prob_recuperacion = 10; 
        limite_vacunas = 4;    
        cout << "\n[!] MODO RAPIDO SELECCIONADO [!]" << endl;
    } else {
        filas = 20; 
        columnas = 20;
        prob_inicial = 5;       
        prob_contagio = 45;     
        prob_recuperacion = 15; 
        limite_vacunas = 12;    
        cout << "\n[*] MODO EXTENDIDO SELECCIONADO [*]" << endl;
    }
    
    generarEscenarioAleatorio(nombre_archivo, filas, columnas, prob_inicial);
    cargarEscenario(nombre_archivo, ciudad);
    
    if (ciudad.empty()) {
        cout << "Error: No se pudo cargar la ciudad o el archivo esta vacio." << endl;
        return 1;
    }

    int poblacion_total = filas * columnas;
    int limite_colapso = poblacion_total / 2; 
    bool ejecutando = true;
    
    int sanos = 0, infectados = 0, recuperados = 0;

    while (ejecutando) {
        sanos = 0; infectados = 0; recuperados = 0;
        for (size_t i = 0; i < ciudad.size(); ++i) {
            for (size_t j = 0; j < ciudad[i].size(); ++j) {
                if (ciudad[i][j] == '.') sanos++;
                else if (ciudad[i][j] == '*') infectados++;
                else if (ciudad[i][j] == '#') recuperados++;
            }
        }

        cout << "\n--- DIA " << dias << " ---" << endl;
        cout << "Estadisticas: " << sanos << " Sanos | " << infectados << " Infectados | " << recuperados << " Inmunes" << endl;
        cout << "Lim. de Colapso del Sist. de Salud: " << limite_colapso << " infectados simultaneos." << endl;
        imprimirMatriz(ciudad);
        
        // Verificar si se gana o no 
        if (infectados >= limite_colapso) {
            cout << "\n=================================================" << endl;
            cout << "  [GAME OVER] EL SISTEMA DE SALUD HA COLAPSADO   " << endl;
            cout << "=================================================" << endl;
            cout << "La infeccion supero el 50% de la ciudad (" << infectados << " personas)." << endl;
            break; 
        }
        else if (infectados == 0 && dias > 0) {
            cout << "\n=================================================" << endl;
            cout << "      [VICTORIA] EL VIRUS HA SIDO ERRADICADO     " << endl;
            cout << "=================================================" << endl;
            cout << "Salvaste la ciudad en " << dias << " dias." << endl;
            break; 
        }

        int opcion = mostrarMenu();
        
        if (opcion == 1) {
            avanzarCiclo(ciudad, dias, prob_contagio, prob_recuperacion);
        } 
        else if (opcion == 2 || opcion == 3) {
            int f, c;
            cout << "Ingrese la fila (0 a " << ciudad.size() - 1 << "): ";
            cin >> f;
            cout << "Ingrese la columna (0 a " << ciudad[0].size() - 1 << "): ";
            cin >> c;
            aplicarIntervencion(ciudad, opcion, f, c);
            avanzarCiclo(ciudad, dias, prob_contagio, prob_recuperacion);
        } 
        else if (opcion == 4) {
            int f, c;
            cout << "--- DESPLIEGUE DE SUPER VACUNA ---" << endl;
            cout << "Limite de alcance para este modo: " << limite_vacunas << " personas." << endl;
            cout << "Ingrese las coordenadas de una persona sana (.)..." << endl;
            cout << "Fila: "; cin >> f;
            cout << "Columna: "; cin >> c;
            
            if (f >= 0 && f < ciudad.size() && c >= 0 && c < ciudad[0].size() && ciudad[f][c] == '.') {
                int total_vacunados = 0;
                superVacunaRecursiva(ciudad, f, c, total_vacunados, limite_vacunas);
                cout << "\n>>> ¡BOMBA MEDICA DESPLEGADA! " << total_vacunados << " personas fueron inmunizadas en cadena. <<<" << endl;
                
                avanzarCiclo(ciudad, dias, prob_contagio, prob_recuperacion);
            } else {
                cout << "Coordenada invalida. Debes apuntar a una persona sana (.). No perdiste el turno." << endl;
            }
        }
        else if (opcion == 5) {
            cout << "\nRendicion aceptada. Saliendo de la simulacion..." << endl;
            ejecutando = false;
        }
        else {
            cout << "Opcion no valida." << endl;
        }
    }

    guardarReporte(dias, sanos, infectados, recuperados);
    cout << "\nReporte guardado con exito. ¡Gracias por jugar!" << endl;
    return 0;
}

// Se implementan las funciones de escenario 

void generarEscenarioAleatorio(const string& nombre_archivo, int filas, int columnas, int prob_inicial) {
    ofstream archivo(nombre_archivo);
    if (archivo.is_open()) {
        for (int i = 0; i < filas; ++i) {
            for (int j = 0; j < columnas; ++j) {
                if (rand() % 100 < prob_inicial) {
                    archivo << '*';
                } else {
                    archivo << '.';
                }
            }
            archivo << endl;
        }
        archivo.close();
    }
}

void cargarEscenario(const string& nombre_archivo, vector<string>& matriz_ciudad) {
    ifstream archivo(nombre_archivo);
    string linea;
    matriz_ciudad.clear(); 
    if (archivo.is_open()) {
        while (getline(archivo, linea)) {
            matriz_ciudad.push_back(linea);
        }
        archivo.close();
    }
}

void imprimirMatriz(const vector<string>& matriz_ciudad) {
    for (size_t i = 0; i < matriz_ciudad.size(); ++i) {
        for (size_t j = 0; j < matriz_ciudad[i].size(); ++j) {
            cout << matriz_ciudad[i][j] << " ";
        }
        cout << "  (Fila " << i << ")" << endl;
    }
}

int mostrarMenu() {
    int opcion;
    cout << "\n=== MENU TACTICO ===" << endl;
    cout << "1. Esperar al siguiente dia (No intervenir)" << endl;
    cout << "2. Vacunar a UNA persona (#)          [Consume 1 turno]" << endl;
    cout << "3. Curar a un infectado (.)           [Consume 1 turno]" << endl;
    cout << "4. SUPER VACUNA EN CADENA (Recursiva) [Consume 1 turno]" << endl;
    cout << "5. Finalizar simulacion / Rendirse" << endl;
    cout << "Seleccione una opcion: ";
    cin >> opcion;
    return opcion;
}

void avanzarCiclo(vector<string>& matriz_ciudad, int& dias, int prob_contagio, int prob_recuperacion) {
    vector<string> nueva_ciudad = matriz_ciudad; 
    int filas = matriz_ciudad.size();
    int cols = matriz_ciudad[0].size();

    for (int i = 0; i < filas; ++i) {
        for (int j = 0; j < cols; ++j) {
            if (matriz_ciudad[i][j] == '.') { 
                int infectados_cerca = 0;
                if (i > 0 && matriz_ciudad[i-1][j] == '*') infectados_cerca++;
                if (i < filas - 1 && matriz_ciudad[i+1][j] == '*') infectados_cerca++;
                if (j > 0 && matriz_ciudad[i][j-1] == '*') infectados_cerca++;
                if (j < cols - 1 && matriz_ciudad[i][j+1] == '*') infectados_cerca++;

                for (int k = 0; k < infectados_cerca; ++k) {
                    if (rand() % 100 < prob_contagio) {
                        nueva_ciudad[i][j] = '*';
                        break;
                    }
                }
            } 
            else if (matriz_ciudad[i][j] == '*') { 
                if (rand() % 100 < prob_recuperacion) {
                    nueva_ciudad[i][j] = '#'; 
                }
            }
        }
    }
    matriz_ciudad = nueva_ciudad;
    dias++;
    cout << "\n>>> El tiempo avanza. El virus se propaga... <<<" << endl;
}

void aplicarIntervencion(vector<string>& matriz_ciudad, int tipo_accion, int fila, int columna) {
    if (fila < 0 || fila >= matriz_ciudad.size() || columna < 0 || columna >= matriz_ciudad[0].size()) {
        cout << "Coordenadas fuera de rango. Perdiste tu turno." << endl;
        return;
    }
    if (tipo_accion == 2) {
        matriz_ciudad[fila][columna] = '#'; 
        cout << "Intervencion aplicada: Zona asegurada." << endl;
    } else if (tipo_accion == 3) {
        matriz_ciudad[fila][columna] = '.'; 
        cout << "Intervencion aplicada: Paciente curado." << endl;
    }
}

void guardarReporte(int dias, int total_sanos, int total_infectados, int total_recuperados) {
    ofstream archivo("reporte_simulacion.txt");
    if (archivo.is_open()) {
        archivo << "=== REPORTE FINAL DE LA SIMULACION ===" << endl;
        archivo << "Dias transcurridos: " << dias << endl;
        archivo << "Total de poblacion sana restante: " << total_sanos << endl;
        archivo << "Total de poblacion infectada: " << total_infectados << endl;
        archivo << "Total de poblacion recuperada/inmune: " << total_recuperados << endl;
        archivo.close();
    }
}

// Funcion recursiva 
void superVacunaRecursiva(vector<string>& matriz_ciudad, int fila, int columna, int& vacunados, int limite_dosis) {
    if (vacunados >= limite_dosis) {
        return;
    }
    if (fila < 0 || fila >= matriz_ciudad.size() || columna < 0 || columna >= matriz_ciudad[0].size()) {
        return; 
    }
    if (matriz_ciudad[fila][columna] != '.') {
        return; 
    }

    // Aplicamos la vacuna
    matriz_ciudad[fila][columna] = '#';
    vacunados++; 

    superVacunaRecursiva(matriz_ciudad, fila + 1, columna, vacunados, limite_dosis);  
    superVacunaRecursiva(matriz_ciudad, fila - 1, columna, vacunados, limite_dosis);  
    superVacunaRecursiva(matriz_ciudad, fila, columna + 1, vacunados, limite_dosis);  
    superVacunaRecursiva(matriz_ciudad, fila, columna - 1, vacunados, limite_dosis);  
}
