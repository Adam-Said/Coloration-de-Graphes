import os
import time
import socket
from subprocess import Popen, PIPE

print("Compilation...")
os.system("mkdir obj")
os.system("mkdir bin")
os.system("make")
print("Démarrage du programme")

s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
s.connect(("8.8.8.8", 80))

print("Nombre de client:  \n")
nbClient = int(input())
print("IP Serveur:  \n")
ipServeur = str(input())
print("Port Serveur: \n")
portServeur = int(input())
print("Port premier client \n")
portClient = int(input())

verboseChoise = input("Mode verbeux ? (y/n) \n")
if verboseChoise == "y":
    verbose = 1
else:
    verbose = 0


print("Lancement de", nbClient , " clients")

for x in range(nbClient) :
  time.sleep(0.05)
  tmpPort = portClient+x+1
  print("[Runner] Démarrage du client" , x , "sur" , str(s.getsockname()[0]) + ":" + str(tmpPort) ,"au serveur")
  call = "./bin/client "+ str(ipServeur) + " " + str(portServeur) + " " + str(s.getsockname()[0]) + " " + str(tmpPort) + " " + str(verbose) + " &"
  os.system(call)
  


