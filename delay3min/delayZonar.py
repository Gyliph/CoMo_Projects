#-------------------------------------------------------------------------------
# Name:        CSV Import From Zonar
# Author:      Rich Buford & Jeff King
# Created:     01/03/2017
# Copyright:   (c) richbuford 2017
#-------------------------------------------------------------------------------

import csv
import time
import datetime
import urllib
import os
import shutil
import sys
import xml.etree.ElementTree as ET

#This class is to be used by the delayed zonar services.
#When you create a new object from this class, you pass in the parameter of the time delay to lag behind in zonar and the path to write .csv outputs to for input into geoevent
#The time delay exists because of the fact that zonar misses uploaded data from transport latency
#Zonar requires a maximum of 5 minutes to collect and save data to the vehicle hardware, and subsequently requires another maximum of 5 minutes to upload the data to Zonar's data endpoints
#Therefore, a short time delay of ~30 seconds would correspond to much more recent AVL data, but much less accuracy of data than a longer time delay of ~3 minutes
#Picking your time delay is problem dependent and requires you to consider whether accuracy or latency is more important

class delayZonar:

    def __init__(self, timeDelay, pathTo):
        self.timeDelay = timeDelay
        self.pathTo = pathTo
        self.timeWindow = 5
        self.auth = {"username":"notset", "password":"notset"}

    def readAuth(self):
        try:
            authfile = open(self.pathTo + "auth.txt", "r+")
            for line in authfile:
                splitline = line.split(":");
                self.auth[splitline[0]] = splitline[1]
            authfile.close()
        except Exception as e:
            self.writeError("Failed pulling auth info from auth file at {1}: {0}\n".format(str(e), str(time.time())))
    
    def looper(self):
        try:
            runlog = open(self.pathTo + "runtime.log", "w+")
            self.readAuth()
            prev_time_file = self.pathTo + "prev_time.log"
            try:
                lookupxml = self.pathTo + "lookupxml.xml"
                xml_url = "https://col2225.zonarsystems.net/interface.php?action=showopen&operation=showassets&username=" + self.auth["username"] + "&password=" + self.auth["password"] + "&format=xml&version=2&logvers=3.6"
                urllib.urlretrieve(xml_url, lookupxml)
                tree = ET.parse(lookupxml)
                root = tree.getroot()
                lookuploc = {}
                lookupstype = {}
                lookupdep = {}
                for member in root.findall('asset'):
                    fleet = member.find('fleet').text
                    location = member.find('location').text
                    subtype = member.find('subtype').text
                    custom = member.find('customdata')
                    lookupdep[fleet] = 'None'
                    for label in custom.findall('label'):
                        dep = label.get('name')
                        val = label.get('value')
                        if(dep == 'Department'):
                            lookupdep[fleet] = val
                    lookuploc[fleet] = location
                    lookupstype[fleet] = subtype
            except Exception as e:
                self.writeError("Failed creating lookup table at {1}: {0}\n".format(str(e), str(time.time())))

            endtime = int(time.time() - self.timeDelay)
            try:
                prev_time = open(prev_time_file, "r+")
                liner = prev_time.readline()
                if(liner == "" or liner == "\n"):
                    starttime = endtime - self.timeWindow
                else:
                    starttime = int(liner)
                prev_time.close()
            except:
                starttime = endtime - self.timeWindow

            tstarttime = str(starttime)
            tendtime = str(endtime)

            try:
                next_time = open(prev_time_file, "w+")
                next_time.write(str(endtime+1))
                next_time.close()
            except Exception as e:
                self.writeError("Failed opening prev_time log at {1}: {0}\n".format(str(e), str(time.time())))

            start = int(time.time())

            req_list = {"action": "showposition",
            "logvers": "3.6",
            "username": self.auth["username"],
            "password": self.auth["password"],
            "operation": "path",
            "format": "csv",
            "version": "2",
            "starttime": tstarttime,
            "endtime": tendtime}

            #WaterLight
            try:
                wlname = "ZonarWaterLight" + tendtime
                wlout1 = self.pathTo + "WaterLight\\Temp\\" + wlname + "_1" + ".csv"
                wlout2 = self.pathTo + "WaterLight\\Temp\\" + wlname + "_2" + ".csv"
                wlout3 = self.pathTo + "WaterLight\\Temp\\" + wlname + "_3" + ".csv"
                wlout4 = self.pathTo + "WaterLight\\Temp\\" + wlname + "_4" + ".csv"
                wlout5 = self.pathTo + "WaterLight\\Temp\\" + wlname + "_5" + ".csv"
                wlout6 = self.pathTo + "WaterLight\\Temp\\" + wlname + ".csv"
                wlout = self.pathTo + "WaterLight\\" + wlname + "" + ".csv"

                req_list["location"] = "Electric Distribution"
                req_params = urllib.urlencode(req_list)
                wlurl1 = "http://col2225.zonarsystems.net/interface.php?%s" % req_params

                req_list["location"] = "Electric Utility Services"
                req_params = urllib.urlencode(req_list)
                wlurl2 = "http://col2225.zonarsystems.net/interface.php?%s" % req_params

                req_list["location"] = "Water Distribution"
                req_params = urllib.urlencode(req_list)
                wlurl3 = "http://col2225.zonarsystems.net/interface.php?%s" % req_params

                req_list["location"] = "Water Light Engineering"
                req_params = urllib.urlencode(req_list)
                wlurl4 = "http://col2225.zonarsystems.net/interface.php?%s" % req_params

                req_list["location"] = "Electric Engineering"
                req_params = urllib.urlencode(req_list)
                wlurl5 = "http://col2225.zonarsystems.net/interface.php?%s" % req_params

                urllib.urlretrieve(wlurl1, wlout1)
                urllib.urlretrieve(wlurl2, wlout2)
                urllib.urlretrieve(wlurl3, wlout3)
                urllib.urlretrieve(wlurl4, wlout4)
                urllib.urlretrieve(wlurl5, wlout5)

                f = wlout1
                csvinputs = [wlout2, wlout3, wlout4, wlout5]
                self.appendCsvs(f, csvinputs, wlout6)
                self.writeToOutput(wlout6, wlout, lookuploc, lookupstype, lookupdep)
                os.remove(wlout1)
                os.remove(wlout3)
                os.remove(wlout2)
                os.remove(wlout4)
                os.remove(wlout5)
                os.remove(wlout6)
                runlog.write("WaterLight Imported\n")
            except Exception as e:
                self.writeError("Failed Waterlight at {1}: {0}\n".format(str(e), str(time.time())))
            #End WaterLight

            #Sewer
            try:
                sewername = "ZonarSewer" + tendtime
                sewerout1 = self.pathTo + "Sewer\\Temp\\" + sewername + "" + ".csv"
                sewerout = self.pathTo + "Sewer\\" + sewername + "" + ".csv"

                req_list["location"] = "Sewer and Stormwater - WWTP"
                req_params = urllib.urlencode(req_list)
                sewerurl = "http://col2225.zonarsystems.net/interface.php?%s" % req_params

                urllib.urlretrieve(sewerurl, sewerout1)
                self.writeToOutput(sewerout1, sewerout, lookuploc, lookupstype, lookupdep)
                runlog.write("Sewer Imported\n")
                os.remove(sewerout1)
            except Exception as e:
                self.writeError("Failed Sewer at {1}: {0}\n".format(str(e), str(time.time())))
            #End Sewer

            #Street
            try:
                streetname = "ZonarStreet" + tendtime
                streetout1 = self.pathTo + "Street\\Temp\\" + streetname + "_1" + ".csv"
                streetout2 = self.pathTo + "Street\\Temp\\" + streetname + "_2" + ".csv"
                streetout3 = self.pathTo + "Street\\Temp\\" + streetname + "_5" + ".csv"
                streetout4 = self.pathTo + "Street\\" + streetname + "" + ".csv"

                req_list["location"] = "Street - Grissum"
                req_params = urllib.urlencode(req_list)
                streeturl1 = "http://col2225.zonarsystems.net/interface.php?%s" % req_params

                req_list["location"] = "Street Sweepers - Grissum"
                req_params = urllib.urlencode(req_list)
                streeturl2 = "http://col2225.zonarsystems.net/interface.php?%s" % req_params

                urllib.urlretrieve(streeturl1, streetout1)
                urllib.urlretrieve(streeturl2, streetout2)

                fstreets = streetout1
                streetinputs = [streetout2]
                self.appendCsvs(fstreets, streetinputs, streetout3)
                self.writeToOutput(streetout3, streetout4, lookuploc, lookupstype, lookupdep)
                os.remove(streetout1)
                os.remove(streetout2)
                os.remove(streetout3)
                runlog.write("Streets Imported\n")
            except Exception as e:
                self.writeError("Failed Street at {1}: {0}\n".format(str(e), str(time.time())))
            #End Street

            #Solid Waste
            try:
                solidwastename = "ZonarSolidWaste" + tendtime
                solidout1 = self.pathTo + "SolidWaste\\Temp\\" + solidwastename + "" + ".csv"
                solidout = self.pathTo + "SolidWaste\\" + solidwastename + "" + ".csv"

                req_list["location"] = "Solid Waste - Grissum"
                req_params = urllib.urlencode(req_list)
                solidurl = "http://col2225.zonarsystems.net/interface.php?%s" % req_params

                urllib.urlretrieve(solidurl, solidout1)
                self.writeToOutput(solidout1, solidout, lookuploc, lookupstype, lookupdep)
                os.remove(solidout1)
                runlog.write("Solid Imported\n")
            except Exception as e:
                self.writeError("Failed Solid Waste at {1}: {0}\n".format(str(e), str(time.time())))
            #End Solid Waste

            endtime2 = int(time.time())
            print tendtime
            print tstarttime
            timedif = endtime2-start
            print timedif

            runlog.write('It took {0} seconds.\n'.format(endtime2-start))
            runlog.close()
        except Exception as e:
            self.writeError("Failed main loop at {1}: {0}\n".format(str(e), str(time.time())))

    #function called by looper to writeoutput for each vehicle to csvs
    def writeToOutput(self, out, write, lookuploc, lookupstype, lookupdep):
        with open(out, 'rb') as fileIn:
            with open(write, 'wb') as fileOut:
                reader = csv.reader(fileIn)
                writer = csv.writer(fileOut)
                rowWriter = []
                row = reader.next()
                row.append('Location')
                row.append('Subtype')
                row.append('Department')
                for x in range(0,12):
                    del row[17]
                rowWriter.append(row)
                for row in reader:
                    if(len(row) == 29):
                        row.append(lookuploc[row[1]])
                        row.append(lookupstype[row[1]])
                        row.append(lookupdep[row[1]])
                        ept = int(row[3])
                        convt = time.strftime('%Y-%m-%d %H:%M:%S', time.localtime(ept))
                        row[2] = convt
                        for x in range(0,12):
                            del row[17]
                        rowWriter.append(row)
                writer.writerows(rowWriter)
        return

    def writeError(self, string):
        try:
            errlog = open(self.pathTo + "errors.log", "a+")
            errlog.write(string)
            errlog.close()
            sys.exit(1)
        except:
            sys.exit(1)

    #function called before writeToOutput in the case that there are more than one
    #csv inputs that must be appended together
    def appendCsvs(self, f, csvinputs, out):
        op = open(out, 'wb')
        output = csv.writer(op, delimiter =',')
        csvfiles = []
        op1 = open(f, 'rb')
        rd = csv.reader(op1, delimiter = ',')
        for row in rd:
            csvfiles.append(row)
        for files in csvinputs:
            op2 = open(files, 'rb')
            rd2 = csv.reader(op2, delimiter = ',')
            rd2.next()
            for row2 in rd2:
                csvfiles.append(row2)
            op2.close()
        output.writerows(csvfiles)
        op.close()
        op1.close()
        return

    def run(self):
        #try:
        #    shutil.rmtree(self.pathTo + "WaterLight")
        #    shutil.rmtree(self.pathTo + "Sewer")
        #    shutil.rmtree(self.pathTo + "Street")
        #    shutil.rmtree(self.pathTo + "SolidWaste")
        #    time.sleep(self.timeWindow)
        #    os.makedirs(self.pathTo + "WaterLight")
        #    os.makedirs(self.pathTo + "Sewer")
        #    os.makedirs(self.pathTo + "Street")
        #    os.makedirs(self.pathTo + "SolidWaste")
        #    os.makedirs(self.pathTo + "WaterLight//Temp")
        #    os.makedirs(self.pathTo + "Sewer//Temp")
        #    os.makedirs(self.pathTo + "Street//Temp")
        #    os.makedirs(self.pathTo + "SolidWaste//Temp")
        #except:
        #    self.writeError("Failed replacing directories at {1}: {0}\n".format(str(e), str(time.time())))
        #    sys.exit(3)

        for x in range(int(3600/self.timeWindow)):
            try:
                self.looper()
                time.sleep(self.timeWindow)
            except Exception as e:
                self.writeError("Failed main at {1}: {0}\n".format(str(e), str(time.time())))




