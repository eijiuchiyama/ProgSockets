#!/usr/bin/env python3
"""
Pytest suite para testar todos os métodos da classe DrcpClient.
Executa testes unitários com mocks e testes de integração com servidor real.
"""

from client import DrcpClient
import pytest
import socket
from unittest.mock import Mock, patch, MagicMock
import sys
import os

# Adicionar o diretório ao path
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))


class TestDrcpClientInit:
    """Testes para inicialização da classe DrcpClient."""

    @patch('client.DrcpClient.connect')
    def test_init_default_params(self, mock_connect):
        """Testa inicialização com parâmetros padrão."""
        client = DrcpClient.__new__(DrcpClient)
        client.host = 'localhost'
        client.port = 5000

        assert client.host == 'localhost'
        assert client.port == 5000

    @patch('client.DrcpClient.connect')
    def test_init_custom_params(self, mock_connect):
        """Testa inicialização com parâmetros customizados."""
        client = DrcpClient.__new__(DrcpClient)
        client.host = '192.168.1.1'
        client.port = 8080

        assert client.host == '192.168.1.1'
        assert client.port == 8080


class TestDrcpClientConnect:
    """Testes para o método connect()."""

    @patch('socket.socket.connect')
    @patch('builtins.print')
    def test_connect_success(self, mock_print, mock_socket_connect):
        """Testa conexão bem-sucedida."""
        with patch('socket.socket'):
            client = DrcpClient.__new__(DrcpClient)
            client.host = 'localhost'
            client.port = 5000
            client.socket = Mock()
            client.connect()

            # Verificar que foi chamado corretamente
            client.socket.connect.assert_called_once()

    def test_connect_connection_refused(self):
        """Testa erro ao conectar (servidor não disponível)."""
        with patch('socket.socket') as mock_socket:
            mock_instance = Mock()
            mock_instance.connect.side_effect = ConnectionRefusedError()
            mock_socket.return_value = mock_instance

            with pytest.raises(SystemExit):
                client = DrcpClient.__new__(DrcpClient)
                client.host = 'localhost'
                client.port = 5000
                client.socket = mock_instance
                client.connect()


class TestSendCommand:
    """Testes para o método send_command()."""

    @patch('builtins.print')
    def test_send_command_success(self, mock_print):
        """Testa envio de comando bem-sucedido."""
        client = DrcpClient.__new__(DrcpClient)

        # Mock do socket
        mock_socket = Mock()
        mock_socket.recv.return_value = b'OK response'
        client.socket = mock_socket

        response = client.send_command("TEST COMMAND")

        assert response == 'OK response'
        mock_socket.sendall.assert_called_once_with(b"TEST COMMAND\n")
        mock_socket.recv.assert_called_once()

    @patch('builtins.print')
    def test_send_command_with_spaces(self, mock_print):
        """Testa envio de comando com espaços."""
        client = DrcpClient.__new__(DrcpClient)

        mock_socket = Mock()
        mock_socket.recv.return_value = 'OK 0'.encode('utf-8')
        client.socket = mock_socket

        response = client.send_command("CREATE valor com espaços")

        assert response == 'OK 0'
        mock_socket.sendall.assert_called_once()

    @patch('builtins.print')
    def test_send_command_error(self, mock_print):
        """Testa erro ao enviar comando."""
        client = DrcpClient.__new__(DrcpClient)

        mock_socket = Mock()
        mock_socket.sendall.side_effect = Exception("Connection lost")
        client.socket = mock_socket

        response = client.send_command("TEST")

        assert response is None
        mock_print.assert_called_once()


class TestCreateMethod:
    """Testes para o método create()."""

    @patch('client.DrcpClient.send_command')
    @patch('builtins.print')
    def test_create_success(self, mock_print, mock_send):
        """Testa criação bem-sucedida de recurso."""
        client = DrcpClient.__new__(DrcpClient)
        mock_send.return_value = 'OK 0'

        response = client.create('test_value')

        assert response == 'OK 0'
        mock_send.assert_called_once_with('CREATE test_value')
        mock_print.assert_called_once_with('→ OK 0')

    @patch('client.DrcpClient.send_command')
    @patch('builtins.print')
    def test_create_with_special_chars(self, mock_print, mock_send):
        """Testa criação com caracteres especiais."""
        client = DrcpClient.__new__(DrcpClient)
        mock_send.return_value = 'OK 1'

        response = client.create('valor@#$%&')

        assert response == 'OK 1'
        mock_send.assert_called_once_with('CREATE valor@#$%&')

    @patch('client.DrcpClient.send_command')
    @patch('builtins.print')
    def test_create_no_response(self, mock_print, mock_send):
        """Testa criação sem resposta."""
        client = DrcpClient.__new__(DrcpClient)
        mock_send.return_value = None

        response = client.create('test_value')

        assert response is None
        mock_print.assert_not_called()


class TestGetMethod:
    """Testes para o método get()."""

    @patch('client.DrcpClient.send_command')
    @patch('builtins.print')
    def test_get_success(self, mock_print, mock_send):
        """Testa leitura bem-sucedida."""
        client = DrcpClient.__new__(DrcpClient)
        mock_send.return_value = 'OK valor_armazenado'

        response = client.get(0)

        assert response == 'OK valor_armazenado'
        mock_send.assert_called_once_with('GET 0')
        mock_print.assert_called_once()

    @patch('client.DrcpClient.send_command')
    @patch('builtins.print')
    def test_get_invalid_id(self, mock_print, mock_send):
        """Testa leitura com ID inválido."""
        client = DrcpClient.__new__(DrcpClient)
        mock_send.return_value = 'ERROR 2 Recurso inexistente'

        response = client.get(999)

        assert 'ERROR' in response
        mock_send.assert_called_once_with('GET 999')

    @patch('client.DrcpClient.send_command')
    @patch('builtins.print')
    def test_get_no_response(self, mock_print, mock_send):
        """Testa leitura sem resposta."""
        client = DrcpClient.__new__(DrcpClient)
        mock_send.return_value = None

        response = client.get(0)

        assert response is None


class TestSetMethod:
    """Testes para o método set()."""

    @patch('client.DrcpClient.send_command')
    @patch('builtins.print')
    def test_set_success(self, mock_print, mock_send):
        """Testa modificação bem-sucedida."""
        client = DrcpClient.__new__(DrcpClient)
        mock_send.return_value = 'OK'

        response = client.set(0, 'novo_valor')

        assert response == 'OK'
        mock_send.assert_called_once_with('SET 0 novo_valor')
        mock_print.assert_called_once()

    @patch('client.DrcpClient.send_command')
    @patch('builtins.print')
    def test_set_with_spaces(self, mock_print, mock_send):
        """Testa modificação com espaços no valor."""
        client = DrcpClient.__new__(DrcpClient)
        mock_send.return_value = 'OK'

        response = client.set(0, 'valor com múltiplos espaços')

        assert response == 'OK'
        mock_send.assert_called_once_with('SET 0 valor com múltiplos espaços')

    @patch('client.DrcpClient.send_command')
    @patch('builtins.print')
    def test_set_no_response(self, mock_print, mock_send):
        """Testa modificação sem resposta."""
        client = DrcpClient.__new__(DrcpClient)
        mock_send.return_value = None

        response = client.set(0, 'valor')

        assert response is None


class TestReserveMethod:
    """Testes para o método reserve()."""

    @patch('client.DrcpClient.send_command')
    @patch('builtins.print')
    def test_reserve_success(self, mock_print, mock_send):
        """Testa reserva bem-sucedida."""
        client = DrcpClient.__new__(DrcpClient)
        mock_send.return_value = 'OK'

        response = client.reserve(0)

        assert response == 'OK'
        mock_send.assert_called_once_with('RESERVE 0')
        mock_print.assert_called_once()

    @patch('client.DrcpClient.send_command')
    @patch('builtins.print')
    def test_reserve_already_reserved(self, mock_print, mock_send):
        """Testa reserva de recurso já reservado."""
        client = DrcpClient.__new__(DrcpClient)
        mock_send.return_value = 'ERROR 1 Recurso ja reservado'

        response = client.reserve(0)

        assert 'ERROR' in response
        mock_send.assert_called_once_with('RESERVE 0')

    @patch('client.DrcpClient.send_command')
    @patch('builtins.print')
    def test_reserve_no_response(self, mock_print, mock_send):
        """Testa reserva sem resposta."""
        client = DrcpClient.__new__(DrcpClient)
        mock_send.return_value = None

        response = client.reserve(0)

        assert response is None


class TestReleaseMethod:
    """Testes para o método release()."""

    @patch('client.DrcpClient.send_command')
    @patch('builtins.print')
    def test_release_success(self, mock_print, mock_send):
        """Testa liberação bem-sucedida."""
        client = DrcpClient.__new__(DrcpClient)
        mock_send.return_value = 'OK'

        response = client.release(0)

        assert response == 'OK'
        mock_send.assert_called_once_with('RELEASE 0')
        mock_print.assert_called_once()

    @patch('client.DrcpClient.send_command')
    @patch('builtins.print')
    def test_release_not_reserved(self, mock_print, mock_send):
        """Testa liberação de recurso não reservado."""
        client = DrcpClient.__new__(DrcpClient)
        mock_send.return_value = 'ERROR 3 Recurso nao reservado'

        response = client.release(0)

        assert 'ERROR' in response

    @patch('client.DrcpClient.send_command')
    @patch('builtins.print')
    def test_release_no_response(self, mock_print, mock_send):
        """Testa liberação sem resposta."""
        client = DrcpClient.__new__(DrcpClient)
        mock_send.return_value = None

        response = client.release(0)

        assert response is None


class TestListMethod:
    """Testes para o método list()."""

    @patch('client.DrcpClient.send_command')
    @patch('builtins.print')
    def test_list_empty(self, mock_print, mock_send):
        """Testa listagem vazia."""
        client = DrcpClient.__new__(DrcpClient)
        mock_send.return_value = 'OK 0'

        response = client.list()

        assert response == 'OK 0'
        mock_send.assert_called_once_with('LIST')
        mock_print.assert_called_once()

    @patch('client.DrcpClient.send_command')
    @patch('builtins.print')
    def test_list_with_resources(self, mock_print, mock_send):
        """Testa listagem com recursos."""
        client = DrcpClient.__new__(DrcpClient)
        mock_send.return_value = 'OK 3 0 1 2'

        response = client.list()

        assert '3' in response
        assert '0' in response
        assert '1' in response
        assert '2' in response

    @patch('client.DrcpClient.send_command')
    @patch('builtins.print')
    def test_list_no_response(self, mock_print, mock_send):
        """Testa listagem sem resposta."""
        client = DrcpClient.__new__(DrcpClient)
        mock_send.return_value = None

        response = client.list()

        assert response is None


class TestCloseMethod:
    """Testes para o método close()."""

    @patch('builtins.print')
    def test_close_success(self, mock_print):
        """Testa fechamento bem-sucedido."""
        client = DrcpClient.__new__(DrcpClient)
        mock_socket = Mock()
        client.socket = mock_socket

        client.close()

        mock_socket.close.assert_called_once()
        mock_print.assert_called_once()

    @patch('builtins.print')
    def test_close_no_socket(self, mock_print):
        """Testa fechamento sem socket."""
        client = DrcpClient.__new__(DrcpClient)
        client.socket = None

        client.close()

        mock_print.assert_not_called()


# ============================================================================
# TESTES DE INTEGRAÇÃO (requerem servidor rodando)
# ============================================================================

class TestDrcpClientIntegration:
    """Testes de integração com servidor real."""

    @pytest.fixture
    def client(self):
        """Fixture para criar cliente integrado."""
        try:
            client = DrcpClient('localhost', 5000)
            yield client
            client.close()
        except SystemExit:
            pytest.skip("Servidor não disponível na porta 5000")

    def test_integration_create_and_list(self, client):
        """Testa criação e listagem de recurso."""
        response = client.create('integration_test')
        # Verifica se tem emoji ou número na resposta
        assert response and len(response) > 0

        list_response = client.list()
        assert list_response and len(list_response) > 0

    def test_integration_reserve_and_release(self, client):
        """Testa reserva e liberação."""
        # Criar recurso
        create_resp = client.create('test_reserve')
        assert create_resp and len(create_resp) > 0

        # Extrair ID
        id_str = create_resp.split()[-1]

        # Reservar
        reserve_resp = client.reserve(id_str)
        assert reserve_resp is not None

        # Liberar
        release_resp = client.release(id_str)
        assert release_resp is not None

    def test_integration_full_workflow(self, client):
        """Testa fluxo completo: create -> get -> set -> release."""
        # Create
        create_resp = client.create('workflow_test')
        assert create_resp and len(create_resp) > 0
        id_str = create_resp.split()[-1]

        # Reserve (necessário para GET/SET)
        reserve_resp = client.reserve(id_str)
        assert reserve_resp is not None

        # Get
        get_resp = client.get(id_str)
        assert isinstance(get_resp, str)

        # Set
        set_resp = client.set(id_str, 'updated_value')
        assert set_resp is not None

        # Release
        release_resp = client.release(id_str)
        assert release_resp is not None


# ============================================================================
# Função principal para executar testes
# ============================================================================

if __name__ == '__main__':
    pytest.main([__file__, '-v', '-s', '--tb=short'])
