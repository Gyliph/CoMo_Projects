#To recompile this file, run the compileEXE.py file
#and the resulting binary will live in the relative dist folder

from delayZonar import delayZonar

d3min = delayZonar(3*60, "C:\\atlas_shared\\AVL\\ZonarHistoricalCSV\\delay3min\\")
d3min.run()
