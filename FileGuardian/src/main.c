#include <stdio.h>
#include <stdlib.h>
#include "monitor.h"

int main() {
    printf("Программа запущена...\n");

    start_monitoring();

    printf("Программа успешно запущена и начала мониторинг...\n");
    printf("Для завершения работы программы нажмите Ctrl+C\n");

    wait_for_sigint();

    printf("Программа завершена.\n");
    
    return 0;
}
