#ifndef CONFIG_H
#define CONFIG_H

#include <stdio.h>
#include <stdlib.h>
#include <prthread.h>
#include "yamlConfigParser.h"
#include "multithreading.h"
#include "directoryAnalyzer.h"

void options(ConfigYAML config);
void display_options(const ConfigYAML *config);

#endif  
