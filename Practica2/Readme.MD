Manual técnico: Aplicación de procesamiento de transacciones multiproceso

Este manual técnico proporciona una explicación completa de la aplicación de procesamiento de transacciones multiproceso. Profundiza en las funcionalidades de la aplicación, las opciones de diseño y los detalles de implementación.

**Público objetivo:**

- Programadores con conocimientos prácticos de C y conceptos básicos de subprocesos múltiples.
- Personas interesadas en comprender el diseño e implementación de un sistema de procesamiento de transacciones.

**Requisitos previos:**

- Familiaridad con el lenguaje de programación C.
- Comprensión básica de conceptos de subprocesos múltiples como mutexes y pthreads.

**Estructura del documento:**

1. Introducción
2. Resumen del sistema
3. Estructuras de datos
4. Procesamiento de transacciones
    - Formato de transacción
    - Análisis de transacciones
    - Realizar transacciones (con sincronización)
5. Manejo e informes de errores
6. Consideraciones de rendimiento y subprocesos múltiples
7. Construyendo y ejecutando la aplicación

**1\. Introducción**

Esta aplicación simula un sistema de procesamiento de transacciones multiproceso para un escenario similar al de un banco. Permite procesar transacciones desde un archivo CSV simultáneamente utilizando múltiples subprocesos. Cada transacción involucra un número de cuenta, un tipo de operación (depósito, retiro o transferencia) y un monto.

**2\. Descripción general del sistema**

La aplicación consta de los siguientes componentes clave:

- **Archivo de transacción:** Un archivo CSV que contiene datos de transacciones en el formato operación, número_cuenta, monto (por ejemplo, depósito, 1001,200).
- **Estructura de datos de la cuenta:** Una estructura de cuenta almacena información sobre una cuenta, incluido su número de cuenta, nombre y saldo actual.
- **Estructura de transacción:** Una estructura de transacción contiene detalles sobre una transacción específica, incluido el tipo de operación, los números de cuenta involucrados y el monto de la transacción.
- **Subprocesos múltiples:** La aplicación utiliza subprocesos POSIX (pthreads) para procesar transacciones al mismo tiempo. Cada subproceso se encarga de analizar y realizar una única transacción desde el archivo CSV.
- **Sincronización:** Los mutex se utilizan para proteger los datos compartidos (saldos de cuentas) durante transacciones simultáneas para evitar condiciones de carrera.
- **Manejo de errores:** La aplicación valida transacciones y maneja errores como formatos no válidos, cuentas inexistentes y saldo insuficiente.

**3\. Estructuras de datos**

### 3.1 Cuenta

C

definición de tipo estructura Cuenta {  
En t número_cuenta;  
carbonizarse nombre\[100\];  
doble balance;  
pthread_mutex_t exclusión mutua;// Mutex para proteger el equilibrio  
} Cuenta;  

- account_no: Identificador entero único para la cuenta.
- nombre: nombre del titular de la cuenta (cadena, limitada a 100 caracteres).
- saldo: Saldo actual de la cuenta (doble).
- mutex: Mutex para sincronizar el acceso al saldo de la cuenta durante las transacciones.

### 3.2 Transacción

C

definición de tipo estructura Transacción {  
Operación tipo operación;  
En t cuenta1;  
En t cuenta2;// Sólo se utiliza para operaciones de transferencia  
doble cantidad;  
} Transacción;  

- operación: Tipo de transacción (depósito, retiro o transferencia). Representado como una enumeración OperationType (definida en otro lugar).
- cuenta1: Número de cuenta involucrada en la transacción.
- cuenta2: Segundo número de cuenta utilizado en operaciones de transferencia (no utilizado para depósitos y retiros).
- importe: El importe monetario asociado con la transacción.

**4\. Procesamiento de transacciones**

**4.1 Formato de transacción**

Las transacciones se almacenan en un archivo CSV y cada línea representa una única transacción. El formato es:

operación, número_cuenta, monto  

- operación: una cadena que representa el tipo de transacción (depósito, retiro o transferencia).
- account_no: un número entero que representa el número de cuenta involucrado en la transacción.
- importe: Un doble que representa el importe monetario de la transacción.

**4.2 Análisis de transacciones**

La aplicación utiliza la función strtok para tokenizar cada línea del archivo CSV, separando los campos mediante comas. Aquí hay un desglose simplificado:

1. Lea una línea del archivo CSV.
2. Utilice strtok para obtener el primer token (operación).
3. Convierta la cadena de operación al valor de enumeración OperationType correspondiente.
4. Utilice strtok nuevamente para obtener el siguiente token (número de cuenta) y convertirlo en un número entero.
5. Para operaciones de transferencia, use strtok una vez más para obtener el segundo número de cuenta y convertirlo a un número entero.
6. Utilice strtok por última vez para obtener el token de cantidad y convertirlo en doble.
7. Valide los datos de la transacción analizada (consulte la siguiente sección).

**4.3 Realizar transacciones (con sincronización)**

Una vez que se analiza y valida una transacción, se pasa a la función perform_transaction. Esta función realiza los siguientes pasos:

1. **Buscar cuentas:** Usa la función find_account para localizar las cuentas involucradas en la transacción. 2.**Sincronización:** Adquiera mutex para ambas cuentas involucradas en la transacción (si es una transferencia). 3.**Realizar operación:** Ejecuta la lógica para el tipo específico de operación: \***Depósito:** Aumenta el saldo de la cuenta1 en la cantidad especificada. \***Retiro:** Si la cuenta1 tiene saldo suficiente, disminuye el saldo de la cuenta1 en la cantidad especificada. \* \*\*Transferencia:\*\* Disminuye el saldo de la cuenta 1 por el monto especificado y aumenta el saldo de la cuenta 2 por el monto especificado\`. 4.**Liberar bloqueos:** Libere los mutex de la cuenta.

**5\. Manejo e informes de errores**

**5.1 Manejo de errores**

La aplicación incorpora un sólido manejo de errores en diferentes etapas:

- **Análisis de transacciones:** Comprueba si hay formatos CSV no válidos, campos faltantes y valores no numéricos.
- **Validación de cuenta:** Verifica que los números de cuenta en la transacción existan.
- **Verificaciones de saldo:** Garantiza que la cuenta principal tenga saldo suficiente para operaciones de retiro y transferencia.
- **Errores del hilo:** Supervisa el estado de devolución de los subprocesos creados en caso de errores de creación de subprocesos.

Si ocurre un error, la aplicación imprime un mensaje de error en la consola y omite la transacción actual.

**5.2 Informes**

La aplicación genera un archivo de registro de transacciones que proporciona información sobre las transacciones procesadas. El informe incluye:

- **Fecha y hora:** La marca de tiempo de cuando se generó el informe.
- **Resumen:** Número total de transacciones cargadas, número de transacciones exitosas y número de transacciones que encontraron errores.
- **Lista de errores:** Si alguna transacción falló, detalles de los errores (número de línea, mensaje de error).
- **Nombre del archivo:** El nombre del archivo del informe incluye una marca de tiempo para una fácil identificación (por ejemplo, transacciones_2024_03_18_16_20_46.log).

**6\. Consideraciones de rendimiento y subprocesos múltiples**

**6.1 subprocesos múltiples**

La aplicación utiliza subprocesos POSIX (pthreads) para mejorar el rendimiento al procesar múltiples transacciones simultáneamente. Cada transacción es manejada por un hilo separado. El programa principal crea estos subprocesos y carga transacciones desde el archivo CSV, distribuyendo la carga de trabajo entre los núcleos disponibles.

**6.2 Sincronización**

Cada cuenta tiene un mutex asociado. Antes de que cualquier hilo modifique el saldo de una cuenta, debe adquirir el mutex. Después de la modificación, el hilo libera el mutex. Este mecanismo de sincronización garantiza la integridad de los datos cuando varios subprocesos intentan actualizar el mismo saldo de cuenta simultáneamente.

**6.3 Factores de desempeño**

Varios factores pueden influir en el rendimiento de la aplicación:

- **Número de hilos:** La experimentación es importante para determinar la cantidad óptima de subprocesos para el sistema y la carga específicos. Demasiados subprocesos pueden generar una sobrecarga debido al cambio de contexto.
- **Estructuras de datos:** Si la búsqueda de cuentas se convierte en un cuello de botella, considere una estructura de datos más eficiente, como una tabla hash, para búsquedas de cuentas más rápidas.
- **Granularidad de sincronización:** Asegúrese de que los mutex se mantengan durante el menor tiempo posible para maximizar la simultaneidad.

**7\. Creación y ejecución de la aplicación**

**7.1 Requisitos previos**

- Compilador de C (como GCC o Clang)
- Biblioteca POSIX Threads (pthreads): generalmente incluida en bibliotecas C estándar

**7.2 Construcción**

Intento

gcc main.c -o principal -lpthread  

- main.c: Su archivo de programa principal.
- \-o main: Nombre del archivo ejecutable de salida.
- \-lpthread: vincula la biblioteca de subprocesos POSIX.

**7.3 Corriendo**

Intento

./transacciones principales.csv  

Reemplace transacciones.csv con la ruta a su archivo de transacciones.

**Notas adicionales**

- **Comentarios de código:** El código a nivel de producción incluiría comentarios completos para explicar funciones, opciones de diseño y posibles optimizaciones para la mantenibilidad.
- **Pruebas:** Un proceso completo de desarrollo de aplicaciones incluye rigurosas pruebas unitarias y de integración.

**¡Avíseme si desea que amplíe secciones específicas, incluya ejemplos de código o analice estrategias de optimización del rendimiento!**