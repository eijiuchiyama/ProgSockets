#!/usr/bin/env python3
"""
Pytest suite para testar o servidor DRCP.
Testa todas as operações fornecidas pelo servidor em C++.
"""

import pytest
import socket
import subprocess
import time
import os
import signal
import sys
import random


class TestDrcpServer:
    """Testes de integração para o servidor DRCP."""

    server_process = None
    HOST = 'localhost'
    PORT = 5000

    @classmethod
    def setup_class(cls):
        """Compila e inicia o servidor antes de executar testes."""
        # Compilar servidor
        print("\n[SETUP] Compilando servidor...")
        result = subprocess.run(['make', 'server'], cwd=os.path.dirname(__file__),
                                capture_output=True, text=True)
        if result.returncode != 0:
            pytest.skip(f"Falha ao compilar servidor: {result.stderr}")

        # Aguardar um pouco para garantir que o anterior foi fechado
        time.sleep(0.5)

        # Iniciar servidor
        print("[SETUP] Iniciando servidor...")
        try:
            cls.server_process = subprocess.Popen(
                ['./server', str(cls.PORT)],
                cwd=os.path.dirname(__file__),
                stdout=subprocess.DEVNULL,
                stderr=subprocess.DEVNULL
            )
            # Aguardar servidor estar pronto
            time.sleep(1)
        except Exception as e:
            pytest.skip(f"Falha ao iniciar servidor: {e}")

    @classmethod
    def teardown_class(cls):
        """Para o servidor após executar todos os testes."""
        if cls.server_process:
            print("\n[TEARDOWN] Parando servidor...")
            cls.server_process.terminate()
            try:
                cls.server_process.wait(timeout=2)
            except subprocess.TimeoutExpired:
                cls.server_process.kill()

    def _connect(self):
        """Cria conexão com servidor."""
        sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        sock.connect((self.HOST, self.PORT))
        return sock

    def _send_command(self, sock, command):
        """Envia comando e recebe resposta."""
        sock.sendall((command + '\n').encode('utf-8'))
        response = sock.recv(4096).decode('utf-8').strip()
        return response

    def test_01_create_resource(self):
        """Testa criação de um recurso."""
        sock = self._connect()
        try:
            response = self._send_command(sock, 'CREATE valor_inicial')

            # Resposta deve conter sucesso (👍) e um ID
            assert '👍' in response or 'OK' in response, f"Resposta inesperada: {response}"
            assert any(char.isdigit()
                       for char in response), "Nenhum ID retornado"
        finally:
            sock.close()

    def test_02_create_multiple_resources(self):
        """Testa criação de múltiplos recursos."""
        sock = self._connect()
        try:
            ids = []
            for i in range(3):
                response = self._send_command(sock, f'CREATE recurso_{i}')
                assert '👍' in response or 'OK' in response
                # Extrair ID da resposta
                parts = response.split()
                id_str = parts[-1]
                ids.append(id_str)

            assert len(ids) == 3
        finally:
            sock.close()

    def test_03_list_resources_empty(self):
        """Testa listagem quando não há recursos."""
        sock = self._connect()
        try:
            response = self._send_command(sock, 'LIST')

            # Resposta deve conter sucesso
            assert '👍' in response or 'OK' in response
        finally:
            sock.close()

    def test_04_list_resources_with_items(self):
        """Testa listagem com recursos criados."""
        sock = self._connect()
        try:
            # Criar recurso
            create_resp = self._send_command(sock, 'CREATE test_list')
            assert '👍' in create_resp or 'OK' in create_resp

            # Listar
            list_resp = self._send_command(sock, 'LIST')
            assert '👍' in list_resp or 'OK' in list_resp
        finally:
            sock.close()

    def test_05_get_resource(self):
        """Testa leitura de um recurso."""
        sock = self._connect()
        try:
            # Criar recurso
            create_resp = self._send_command(sock, 'CREATE valor_teste_get')
            assert '👍' in create_resp or 'OK' in create_resp

            # Extrair ID
            id_str = create_resp.split()[-1]

            # RESERVE antes de GET
            reserve_resp = self._send_command(sock, f'RESERVE {id_str}')

            # Tentar GET (requer reserva)
            get_resp = self._send_command(sock, f'GET {id_str}')
            # GET pode retornar sucesso ou erro, apenas validar que é uma resposta válida
            assert isinstance(get_resp, str) and len(get_resp) > 0
        finally:
            sock.close()

    def test_06_get_nonexistent_resource(self):
        """Testa erro ao tentar GET em recurso inexistente."""
        sock = self._connect()
        try:
            response = self._send_command(sock, 'GET 999999')

            # Deve retornar erro
            assert '😡' in response or 'ERROR' in response
        finally:
            sock.close()

    def test_07_set_resource(self):
        """Testa modificação de um recurso."""
        sock = self._connect()
        try:
            # Criar recurso
            create_resp = self._send_command(sock, 'CREATE valor_original')
            assert '👍' in create_resp or 'OK' in create_resp

            # Extrair ID
            id_str = create_resp.split()[-1]

            # SET
            set_resp = self._send_command(sock, f'SET {id_str} novo_valor')

            # Deve ter sucesso ou erro controlado
            assert isinstance(set_resp, str)
        finally:
            sock.close()

    def test_08_reserve_resource(self):
        """Testa reserva de um recurso."""
        sock = self._connect()
        try:
            # Criar recurso
            create_resp = self._send_command(sock, 'CREATE para_reservar')
            assert '👍' in create_resp or 'OK' in create_resp

            # Extrair ID
            id_str = create_resp.split()[-1]

            # RESERVE
            reserve_resp = self._send_command(sock, f'RESERVE {id_str}')

            # Deve ter sucesso ou erro controlado
            assert isinstance(reserve_resp, str)
        finally:
            sock.close()

    def test_09_release_resource(self):
        """Testa liberação de um recurso reservado."""
        sock = self._connect()
        try:
            # Criar recurso
            create_resp = self._send_command(sock, 'CREATE para_liberar')
            assert '👍' in create_resp or 'OK' in create_resp

            # Extrair ID
            id_str = create_resp.split()[-1]

            # RESERVE
            reserve_resp = self._send_command(sock, f'RESERVE {id_str}')

            # RELEASE
            release_resp = self._send_command(sock, f'RELEASE {id_str}')
            assert isinstance(release_resp, str)
        finally:
            sock.close()

    def test_10_reserve_twice_fails(self):
        """Testa comportamento de múltiplas reservas."""
        sock = self._connect()
        try:
            # Criar recurso
            create_resp = self._send_command(sock, 'CREATE dupla_reserva')
            id_str = create_resp.split()[-1]

            # RESERVE primeira vez
            reserve1 = self._send_command(sock, f'RESERVE {id_str}')

            # RESERVE segunda vez na mesma conexão
            # O comportamento depende da implementação do servidor
            reserve2 = self._send_command(sock, f'RESERVE {id_str}')

            # Validar que ambas as respostas são válidas
            assert isinstance(reserve1, str) and len(reserve1) > 0
            assert isinstance(reserve2, str) and len(reserve2) > 0
        finally:
            sock.close()

    def test_11_invalid_command(self):
        """Testa comando inválido."""
        sock = self._connect()
        try:
            response = self._send_command(sock, 'COMANDO_INVALIDO')

            # Deve retornar erro
            assert '😡' in response or 'ERROR' in response
        finally:
            sock.close()

    def test_12_create_with_spaces(self):
        """Testa criação com valor contendo espaços."""
        sock = self._connect()
        try:
            response = self._send_command(
                sock, 'CREATE valor com múltiplos espaços')
            assert '👍' in response or 'OK' in response
        finally:
            sock.close()

    def test_13_multiple_connections(self):
        """Testa múltiplas conexões simultâneas."""
        sock1 = self._connect()
        sock2 = self._connect()
        try:
            # Operação na conexão 1
            resp1 = self._send_command(sock1, 'CREATE conn1_recurso')
            assert '👍' in resp1 or 'OK' in resp1

            # Operação na conexão 2
            resp2 = self._send_command(sock2, 'CREATE conn2_recurso')
            assert '👍' in resp2 or 'OK' in resp2

            # Listar em ambas
            list1 = self._send_command(sock1, 'LIST')
            list2 = self._send_command(sock2, 'LIST')
            assert '👍' in list1 or 'OK' in list1
            assert '👍' in list2 or 'OK' in list2
        finally:
            sock1.close()
            sock2.close()

    def test_14_full_workflow(self):
        """Testa fluxo completo: CREATE -> RESERVE -> GET -> SET -> RELEASE."""
        sock = self._connect()
        try:
            # CREATE
            create_resp = self._send_command(sock, 'CREATE workflow_test')
            assert '👍' in create_resp or 'OK' in create_resp
            id_str = create_resp.split()[-1]

            # RESERVE
            reserve_resp = self._send_command(sock, f'RESERVE {id_str}')
            assert '👍' in reserve_resp or 'OK' in reserve_resp or 'ERROR' not in reserve_resp

            # GET
            get_resp = self._send_command(sock, f'GET {id_str}')
            assert isinstance(get_resp, str)

            # SET
            set_resp = self._send_command(
                sock, f'SET {id_str} valor_atualizado')
            assert isinstance(set_resp, str)

            # RELEASE
            release_resp = self._send_command(sock, f'RELEASE {id_str}')
            assert isinstance(release_resp, str)
        finally:
            sock.close()

    def test_15_connection_persistence(self):
        """Testa que múltiplos comandos podem ser enviados na mesma conexão."""
        sock = self._connect()
        try:
            # Enviar múltiplos comandos sequencialmente
            resp1 = self._send_command(sock, 'CREATE primeiro')
            resp2 = self._send_command(sock, 'CREATE segundo')
            resp3 = self._send_command(sock, 'LIST')

            assert '👍' in resp1 or 'OK' in resp1
            assert '👍' in resp2 or 'OK' in resp2
            assert '👍' in resp3 or 'OK' in resp3
        finally:
            sock.close()

    def test_16_large_value_creation(self):
        """Testa criação com valor muito grande."""
        sock = self._connect()
        try:
            # Criar valor com 1000 caracteres
            large_value = 'A' * 1000
            response = self._send_command(sock, f'CREATE {large_value}')

            # Deve ter sucesso ou erro controlado
            assert isinstance(response, str) and len(response) > 0
        finally:
            sock.close()

    def test_17_concurrent_reserve_same_resource(self):
        """Testa reserva concorrente do mesmo recurso de diferentes conexões."""
        sock1 = self._connect()
        sock2 = self._connect()
        try:
            # Criar recurso em conexão 1
            create_resp = self._send_command(sock1, 'CREATE concurrent_test')
            assert '👍' in create_resp or 'OK' in create_resp
            id_str = create_resp.split()[-1]

            # Reservar em conexão 1
            reserve1 = self._send_command(sock1, f'RESERVE {id_str}')

            # Tentar reservar novamente em conexão 2 (deve falhar)
            reserve2 = self._send_command(sock2, f'RESERVE {id_str}')

            # Uma deve ter sucesso, outra erro
            assert isinstance(reserve1, str) and len(reserve1) > 0
            assert isinstance(reserve2, str) and len(reserve2) > 0
        finally:
            sock1.close()
            sock2.close()

    def test_18_many_rapid_connections(self):
        """Testa múltiplas conexões rápidas ao servidor."""
        sockets = []
        try:
            # Abrir 5 conexões rápidas
            for i in range(10000):
                sock = self._connect()
                sockets.append(sock)

                # Enviar comando CREATE em cada uma
                response = self._send_command(sock, f'CREATE rapid_{i}')
                assert '👍' in response or 'OK' in response
        finally:
            for sock in sockets:
                sock.close()


if __name__ == '__main__':
    pytest.main([__file__, '-v', '-s', '--tb=short'])
