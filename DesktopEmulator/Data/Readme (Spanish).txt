============================================================
        Archivo README para el emulador de Vircon32
           (versión 26.02.18) escrito por Carra
============================================================

¿Qué es esto?

    Esto es un emulador de Vircon32, una consola virtual de 32
    bits, para PC. Te permite jugar a juegos de Vircon32 en tu
    ordenador.

    La versión actual se considera terminada, salvo cualquier
    corrección de errores que se pueda necesitar en el futuro. 
    Ya soporta todas las características de la consola, y el
    emulador ya tiene todas las funciones extra que estaban
    previstas (configuraciones, interfaz gráfica, etc).

------------------------------------------------------------

Instalación 

    Según el sistema operativo que uses el proceso para poder
    ejecutar el emulador será distinto:
    
    Windows:
    --------
    Basta con descomprimir el ZIP en cualquier carpeta, ya
    que incluye todas las librerías DLL necesarias.
      
    Linux:
    --------
    Instala el paquete DEB o RPM (según tu sistema) usando
    los comandos que correspondan:
      DEB: (1) Instalar paquete: sudo dpkg -i <paquete>
           (2) Instalar dependencias: sudo apt-get install -f
      RPM: Instalar el paquete junto con sus dependencias:
           sudo yum localinstall <paquete>
              
    MacOS:
    --------
    Primero descomprime el ZIP a cualquier carpeta con permisos
    de escritura. Para instalar las dependencias se usa el
    gestor de paquetes Homebrew (se puede instalar desde su web).
    Los comandos para instalarlas son:
      brew install sdl2
      brew install libpng
    
------------------------------------------------------------

Ventana del programa 

    Normalmente la interfaz gráfica está escondida, pero se
    muestra cuando el ratón está sobre la ventana del emulador.
    También se puede mostrar manualmente con la tecla escape.

    Usa el ratón para navegar por los menús y usar las opciones.

------------------------------------------------------------

Cómo cargar juegos

    Esta versión del emulador sólo soporta cargar un juego al
    iniciar, a través de la línea de comando. Para ello, pasa
    la ruta de la ROM del juego como único argumento del
    programa.
    
------------------------------------------------------------

Controles

    Por defecto sólo el mando 1 está conectado, y está mapeado
    al teclado de la siguiente forma:

      - Cruceta: Teclas de dirección (flechas)
      - Botones L,R: Teclas 'Q','W'
      - Botones Y,X: Teclas 'A','S'
      - Botones B,A: Teclas 'Z','X'
      - Botón Start: Tecla 'Return'
  
------------------------------------------------------------

Novedades en la versión 26.02.18
    
  - Arreglado un error que impedía cambiar la posición de
    reproducción en los canales de sonido.

------------------------------------------------------------

Licencia
    
    Este programa es gratuito y de código abierto. Se ofrece bajo
    la licencia BSD de 3 cláusulas, cuyo texto completo es el
    siguiente:
    
    Copyright 2021-2026 Carra.
    Todos los derechos reservados.
    
    La redistribución y el uso en las formas de código fuente y
    binario, con o sin modificaciones, están permitidos siempre
    que se cumplan las siguientes condiciones:

    1. Las redistribuciones del código fuente deben conservar el
    aviso de copyright anterior, esta lista de condiciones y el
    siguiente descargo de responsabilidad.
    
    2. Las redistribuciones en formato binario deben reproducir
    el aviso de copyright anterior, esta lista de condiciones y
    el siguiente descargo de responsabilidad en la documentación
    y/u otros materiales suministrados con la distribución.
    
    3. Ni el nombre de los titulares de derechos de autor ni los
    nombres de sus colaboradores pueden usarse para apoyar o
    promocionar productos derivados de este software sin permiso
    específico previo y por escrito.

    ESTE SOFTWARE SE SUMINISTRA POR LOS PROPIETARIOS DEL
    COPYRIGHT Y COLABORADORES “COMO ESTÁ” Y CUALQUIER GARANTÍAS
    EXPRESA O IMPLÍCITA, INCLUYENDO, PERO NO LIMITADO A, LAS
    GARANTÍAS IMPLÍCITAS DE COMERCIALIZACIÓN Y APTITUD PARA UN
    PROPÓSITO PARTICULAR SON RECHAZADAS. EN NINGÚN CASO LOS
    PROPIETARIOS DEL COPYRIGHT Y COLABORADORES SERÁN RESPONSABLES
    POR NINGÚN DAÑO DIRECTO, INDIRECTO, INCIDENTAL, ESPECIAL,
    EJEMPLAR O COSECUENCIAL (INCLUYENDO, PERO NO LIMITADO A, LA
    ADQUISICIÓN O SUSTITUCIÓN DE BIENES O SERVICIOS; LA PÉRDIDA
    DE USO, DE DATOS O DE BENEFICIOS; O INTERRUPCIÓN DE LA
    ACTIVIDAD EMPRESARIAL) O POR CUALQUIER TEORÍA DE
    RESPONSABILIDAD, YA SEA POR CONTRATO, RESPONSABILIDAD ESTRICTA
    O AGRAVIO (INCLUYENDO NEGLIGENCIA O CUALQUIER OTRA CAUSA) QUE
    SURJA DE CUALQUIER MANERA DEL USO DE ESTE SOFTWARE, INCLUSO SI
    SE HA ADVERTIDO DE LA POSIBILIDAD DE TALES DAÑOS.
