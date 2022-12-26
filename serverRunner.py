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

verboseChoise = input("Mode verbeux ? (y/n) \n")
if verboseChoise == "y":
    verbose = 1
else:
    verbose = 0

file = '"' + fileName + '"'
print("[Runner] Démarrage du serveur sur " + str(port))
call = "./bin/serverMultiplex " + str(port) + " " + file + " " + str(verbose) + " &"
os.system(call)
