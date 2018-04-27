#To recompile this file, run the compileEXE.py file
#and the resulting binary will live in the relative dist folder

from delayZonar import delayZonar

d30s = delayZonar(30, "C:\\atlas_shared\\AVL\\ZonarHistoricalCSV\\delay30s\\")
d30s.run()

