import os
import time

print("Compilation...")
os.system("mkdir obj")
os.system("mkdir bin")
os.system("make")
print("Démarrage du programme")

print("Fichier à parser \n")
fileName = str(input())
print("Port serveur \n")
port = int(input())

f = open("Files/"+fileName, "r")
nbNode = 0
for x in f:
  if x.startswith("p"):
        nbNode = int(x.split(" ")[2])
        break

print("Besoin de",nbNode , "clients")

file = '"' + fileName + '"'
print("Démarrage du serveur sur " + str(port))
call = "./bin/serverMultiplex " + str(port) + " " + file +" &"
os.system(call)
time.sleep(1)
for x in range(0,nbNode) :
  time.sleep(0.2)
  tmpPort = port+x+1
  print("Démarrage du client" , x , "sur" , str(tmpPort) ,"au serveur", str(port))
  call = "./bin/client 127.0.0.1 " + str(port) + " " + str(tmpPort) +" &"
  os.system(call)
