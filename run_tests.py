#!/usr/bin/env python3
import socket
import threading
import time
import sys

SERVER_IP = '127.0.0.1'
SERVER_PORT = 9000


def send_recv(sock, msg):
    try:
        sock.sendall(msg.encode('utf-8'))
        resp = sock.recv(4096).decode('utf-8')
        return resp.strip()
    except Exception as e:
        return str(e)


def connect_server():
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    s.connect((SERVER_IP, SERVER_PORT))
    return s


def test_concorrencia():
    print("\n" + "="*50)
    print("Teste 1: Concorrência (2 clientes reservando o mesmo recurso)")
    print("="*50)
    try:
        s_setup = connect_server()
        resp = send_recv(s_setup, "CREATE RecursoConcorrencia\n")
        print(f"[Setup] CREATE: {resp}")

        if "👍" not in resp:
            print("Erro ao criar recurso.")
            return

        res_id = resp.split(' ')[1]

        results = []
        barrier = threading.Barrier(2)

        def client_reserve(client_name):
            try:
                s = connect_server()
                barrier.wait()

                resp = send_recv(s, f"RESERVE {res_id}\n")
                results.append((client_name, resp))

                barrier.wait()
                s.close()
            except Exception as e:
                results.append((client_name, f"Erro: {e}"))

        # Inicia duas threads concorrentes
        t1 = threading.Thread(target=client_reserve, args=("Cliente A",))
        t2 = threading.Thread(target=client_reserve, args=("Cliente B",))

        t1.start()
        t2.start()

        t1.join()
        t2.join()

        for client, r in results:
            print(f"[{client}] Resposta do RESERVE: {r}")

        print("\n--> Resultado Esperado:")
        print("    Apenas um cliente deve receber '👍' (Sucesso).")
        print("    O outro deve receber '😡 1 Recurso já reservado'.")

        s_setup.close()
    except Exception as e:
        print(f"Falha no teste: {e}")


def test_falha():
    print("\n" + "="*50)
    print("Teste 2: Tratamento de Falhas (Desconexão Abrupta)")
    print("="*50)
    try:
        s_setup = connect_server()
        resp = send_recv(s_setup, "CREATE RecursoFalha\n")
        print(f"[Setup] CREATE: {resp}")

        if "👍" not in resp:
            print("Erro ao criar recurso.")
            return

        res_id = resp.split(' ')[1]

        s_falho = connect_server()
        resp_reserve = send_recv(s_falho, f"RESERVE {res_id}\n")
        print(f"[Cliente Falho] RESERVE: {resp_reserve}")

        print(
            "[Cliente Falho] Simulando fechamento abrupto de conexão (socket.close())...")
        s_falho.close()

        time.sleep(0.5)

        s_novo = connect_server()
        print(
            f"[Cliente Novo] Tentando reservar o recurso {res_id} após a falha...")
        resp_novo = send_recv(s_novo, f"RESERVE {res_id}\n")
        print(f"[Cliente Novo] Resposta do RESERVE: {resp_novo}")

        print("\n--> Resultado Esperado:")
        print(
            "    Cliente Novo deve receber '👍' (Sucesso), provando que o servidor liberou")
        print("    automaticamente o recurso quando o Cliente Falho se desconectou.")

        s_novo.close()
        s_setup.close()
    except Exception as e:
        print(f"Falha no teste: {e}")


def test_carga():
    print("\n" + "="*50)
    print("Teste 3: Carga (10 clientes simultâneos)")
    print("="*50)

    num_clientes = 500
    sucessos = 0
    lock = threading.Lock()

    def client_task(client_id):
        nonlocal sucessos
        try:
            s = connect_server()

            resp = send_recv(s, f"CREATE Rec_Cliente_{client_id}\n")
            if "👍" not in resp:
                return
            res_id = resp.split(' ')[1]

            resp = send_recv(s, f"RESERVE {res_id}\n")
            if "👍" not in resp:
                return

            resp = send_recv(s, f"SET {res_id} ValorAlterado_{client_id}\n")
            if "👍" not in resp:
                return

            resp = send_recv(s, f"GET {res_id}\n")
            if "👍" not in resp:
                return

            resp = send_recv(s, f"LIST\n")
            if "👍" not in resp:
                return

            resp = send_recv(s, f"RELEASE {res_id}\n")
            if "👍" not in resp:
                return

            with lock:
                sucessos += 1
            s.close()
        except Exception as e:
            print(f"Erro na thread {client_id}: {e}")

    threads = []
    print(f"Iniciando {num_clientes} threads simultâneas.")
    print("Cada thread executará: CREATE -> RESERVE -> SET -> GET -> LIST -> RELEASE")

    start_time = time.time()
    for i in range(num_clientes):
        t = threading.Thread(target=client_task, args=(i,))
        threads.append(t)
        t.start()

    for t in threads:
        t.join()
    end_time = time.time()

    print(
        f"\nResultado da Carga: [{sucessos}/{num_clientes}] clientes completaram com sucesso.")
    print(f"Tempo total de execução: {end_time - start_time:.4f} segundos")
    print("\n--> Resultado Esperado:")
    print(f"    {num_clientes}/{num_clientes} devem completar com sucesso, sem travamento do servidor (deadlock) ou falhas de socket.")


if __name__ == '__main__':
    if len(sys.argv) > 1:
        SERVER_IP = int(sys.argv[1])
    if len(sys.argv) > 2:
        SERVER_PORT = int(sys.argv[2])

    print(
        f"Iniciando testes automáticos no servidor {SERVER_IP}:{SERVER_PORT}")
    print("Por favor, garanta que o servidor já está rodando em outro terminal (ex: ./server 8080).")

    try:
        s = connect_server()
        s.close()
    except ConnectionRefusedError:
        print(
            f"\nNão foi possível conectar ao servidor em {SERVER_IP}:{SERVER_PORT}.")
        print("Inicie o servidor primeiro antes de rodar os testes!")
        sys.exit(1)

    test_concorrencia()
    test_falha()
    test_carga()
    print("\n" + "="*50)
    print("Fim dos testes.")
    print("="*50)
