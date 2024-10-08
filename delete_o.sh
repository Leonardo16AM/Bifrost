#!/bin/bash
# Este script elimina todos los archivos .o en una carpeta recursivamente

find . -type f -name "*.o" -exec rm -f {} \;

echo "Se han eliminado todos los archivos .o."
