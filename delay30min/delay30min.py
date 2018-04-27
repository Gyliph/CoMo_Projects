#To recompile this file, run the compileEXE.py file
#and the resulting binary will live in the relative dist folder

from delayZonar import delayZonar

d30min = delayZonar(30*60, "C:\\atlas_shared\\AVL\\ZonarHistoricalCSV\\delay30min\\")
d30min.run()
