#!/usr/bin/env python3
import sys
import json
import requests

def main():
    if len(sys.argv) < 2:
        print("Uso: python call_llm.py \"Tu prompt aquí\"", file=sys.stderr)
        sys.exit(1)
    
    user_prompt = sys.argv[1]

    url = "http://127.0.0.1:1234/v1/chat/completions"
    headers = {
        "Content-Type": "application/json"
    }
    data = {
        "model": "lmstudio-community/Meta-Llama-3.1-8B-Instruct-GGUF/Meta-Llama-3.1-8B-Instruct-Q6_K.gguf",
        "messages": [
            { "role": "system", "content": 
            """Te daré una entrada de texto de un usuario, y tu tarea es extraer la siguiente información, asegurándote de devolverla 
            en el formato exacto que te indicaré.\n\n
            Datos a extraer:\n
            1. ¿El usuario quiere cargar un archivo? Si es así, devuelve el nombre del archivo en el formato `archivo: nombre.csv`.\n
            2. La cantidad de personas, en el formato `personas: valor`.\n
            3. La cantidad de rutas, en el formato `rutas: valor`.\n
            4. La cantidad de iteraciones para PSO, en el formato `iteraciones: valor`.\n
            5. La cantidad de partículas para PSO, en el formato `particulas: valor`.\n\n
            Si no se proporcionan algunos de estos valores, infiere los siguientes valores predeterminados:\n
             - personas: 500\n
             - iteraciones: 100\n
             - rutas: 10\n
             - particulas: 30\n\n
            Es muy importante que devuelvas la respuesta **exactamente** en el siguiente formato (sin saltos de línea y con los mismos nombres de campo):\n
            `archivo: nombre.csv, personas: valor, rutas: valor, iteraciones: valor, particulas: valor`. SI por alguna razon te faltan datos para xompletar, simplemente pon los predeterminados, no digas nada que no  cumpla el formato dicho anteriormente""" },
            { "role": "user", "content": user_prompt }
        ],
        "temperature": 0.7,
        "max_tokens": -1,
        "stream": False  # Para simplificar, se establece en False
    }

    try:
        response = requests.post(url, headers=headers, data=json.dumps(data))
        response.raise_for_status()
        response_json = response.json()
        # Extraer el contenido de la respuesta
        if 'choices' in response_json and len(response_json['choices']) > 0:
            reply = response_json['choices'][0]['message']['content']
            print(reply)
        else:
            print("No se recibió respuesta del LLM.", file=sys.stderr)
            sys.exit(1)
    except requests.exceptions.RequestException as e:
        print(f"Error al llamar al API del LLM: {e}", file=sys.stderr)
        sys.exit(1)

if __name__ == "__main__":
    main()
