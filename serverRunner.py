import os
import time
from subprocess import Popen, PIPE

print("Compilation...")
os.system("mkdir obj")
os.system("mkdir bin")
os.system("make")
print("Démarrage du programme")

print("Fichier à parser \n")
fileName = str(input())
print("Port serveur \n")
port = int(input())

file = '"' + fileName + '"'
print("[Runner] Démarrage du serveur sur " + str(port))
call = "./bin/serverMultiplex " + str(port) + " " + file +" &"
os.system(call)
