# EP 1 - Servidor DRCP

Servidor de gerenciamento de recursos distribuído utilizando protocolo DRCP (Distributed Resource Control Protocol) sobre TCP.

## 📋 Descrição

Este projeto implementa um servidor multi-threaded que gerencia recursos compartilhados. Suporta operações de criação, reserva, obtenção, modificação e liberação de recursos através de um protocolo de comunicação custom.

### Características

- **Multi-threaded**: Suporta múltiplas conexões simultâneas
- **Thread-safe**: Sincronização via semáforos POSIX
- **Logging detalhado**: Rastreamento de operações para debug
- **Protocolo DRCP**: 6 operações (CREATE, GET, SET, LIST, RESERVE, RELEASE)
- **Tratamento de falhas**: Limpeza automática de recursos para conexões fechadas

---

## 🔧 Requisitos

- **GCC** (com suporte a C++11 ou superior)
- **Make**
- **POSIX pthreads** (incluído em sistemas Unix/Linux)

### Verificar Instalação

```bash
# Verificar GCC
gcc --version

# Verificar Make
make --version
```

---

## 🏗️ Compilação

### Compilar Tudo (Servidor + Cliente)

```bash
make
```

Isso gera dois binários:

- `server` - Servidor DRCP
- `client` - Cliente de teste/exemplo

### Compilar Apenas o Servidor

```bash
make server
```

### Compilar Apenas o Cliente

```bash
make client
```

### Limpar Binários

```bash
make clean
```

---

## ▶️ Execução

### Executar o Servidor

**A porta deve ser especificada como argumento:**

```bash
./server <porta>
```

Exemplos:

```bash
# Porta 9000
./server 9000
```

**Exemplo de saída:**

```
[Servidor no ar. Aguardando conexões na porta 9999]
```

### Executar o Cliente

Em outro terminal:

```bash
./client
```

O cliente se conectará ao servidor na porta configurada e executará operações de exemplo.

## 📝 Exemplo de Uso

### Terminal 1 - Servidor

```bash
$ make server
$ ./server 9999
[Servidor no ar. Aguardando conexões na porta 9999]
```

### Terminal 2 - Cliente

```bash
$ make client
$ ./client
Conectado ao servidor
Enviando: CREATE test 123
Resposta: OK
Enviando: GET test
Resposta: 123
```

### Terminal 2 - Com Netcat (nc)

Você também pode se conectar ao servidor usando **netcat** para testes manuais:

```bash
nc localhost 9000
```

Depois de conectado, envie comandos DRCP diretamente:

```
CREATE recurso1 valor_inicial
GET recurso1
SET recurso1 novo_valor
RESERVE recurso1
RELEASE recurso1
LIST
```

**Exemplo completo:**

```bash
$ nc localhost 9999
CREATE contador 0
GET contador
SET contador 10
RESERVE contador
RELEASE contador
LIST
```

**Exemplo de saída:**

```
CREATE contador 0
OK
GET contador
10
SET contador 10
OK
RESERVE contador
OK
RELEASE contador
OK
LIST
contador
```
