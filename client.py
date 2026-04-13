#!/usr/bin/env python3
"""
Client TCP simples para comunicação com o servidor de recursos.
Conecta ao servidor e permite enviar comandos interativamente.
"""

import socket
import sys


class DrcpClient:
    socket: socket.socket

    def __init__(self, host='localhost', port=5000):
        """Inicializa cliente e conecta ao servidor."""
        self.host = host
        self.port = port
        self.connect()

    def connect(self):
        """Conecta ao servidor TCP."""
        try:
            self.socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            self.socket.connect((self.host, self.port))
            print(f"✓ Conectado ao servidor {self.host}:{self.port}")
        except ConnectionRefusedError:
            print(
                f"✗ Erro: Não foi possível conectar a {self.host}:{self.port}")
            sys.exit(1)
        except Exception as e:
            print(f"✗ Erro de conexão: {e}")
            sys.exit(1)

    def send_command(self, comando):
        """Envia comando para o servidor e recebe resposta."""
        try:
            # Enviar comando (com quebra de linha)
            self.socket.sendall((comando + '\n').encode('utf-8'))

            # Receber resposta
            resposta = self.socket.recv(4096).decode('utf-8')
            return resposta.strip()
        except Exception as e:
            print(f"✗ Erro ao comunicar com servidor: {e}")
            return None

    def create(self, valor):
        """CREATE <valor> - Criar novo recurso com valor."""
        comando = f"CREATE {valor}"
        resposta = self.send_command(comando)
        if resposta:
            print(f"→ {resposta}")
            return resposta

    def get(self, id_recurso):
        """GET <id> - Obter valor de um recurso."""
        comando = f"GET {id_recurso}"
        resposta = self.send_command(comando)
        if resposta:
            print(f"→ {resposta}")
            return resposta

    def set(self, id_recurso, valor):
        """SET <id> <valor> - Modificar valor de um recurso."""
        comando = f"SET {id_recurso} {valor}"
        resposta = self.send_command(comando)
        if resposta:
            print(f"→ {resposta}")
            return resposta

    def reserve(self, id_recurso):
        """RESERVE <id> - Reservar um recurso."""
        comando = f"RESERVE {id_recurso}"
        resposta = self.send_command(comando)
        if resposta:
            print(f"→ {resposta}")
            return resposta

    def release(self, id_recurso):
        """RELEASE <id> - Liberar um recurso."""
        comando = f"RELEASE {id_recurso}"
        resposta = self.send_command(comando)
        if resposta:
            print(f"→ {resposta}")
            return resposta

    def list(self):
        """LIST - Listar todos os recursos."""
        comando = "LIST"
        resposta = self.send_command(comando)
        if resposta:
            print(f"→ {resposta}")
            return resposta

    def close(self):
        """Fecha a conexão com o servidor."""
        if self.socket:
            self.socket.close()
            print("✓ Desconectado")
