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
            { "role": "system", "content": "Always answer in rhymes." },
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
