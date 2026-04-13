---
marp: true
theme: default
paginate: true
size: 16:9
style: |
  section {
    font-size: 20px;
    padding: 60px;
  }
  h1 {
    font-size: 48px;
  }
  h2 {
    font-size: 36px;
  }
  code {
    font-size: 14px;
  }
  table {
    font-size: 18px;
  }
---

# ProgSockets

## Distributed Resource Control Protocol (DRCP)

**Felipe Kelemen | Gustavo Bastos | Lucas Uchiyama | Octavio Carneiro**

MAC0352 - EP1

---

## Visão Geral do Projeto

- **Objetivo**: Sistema distribuído para controle de recursos via rede
- **Arquitetura**: Cliente-Servidor
- **Protocolo de Aplicação**: DRCP (próprio)
- **Protocolo de Transporte**: TCP
- **Linguagem**: C++
- **Concorrência**: Threads (pthread)
- **Sincronização**: Semáforos

---

## Arquitetura do Sistema

**Cliente-Servidor com DRCP sobre TCP**

```
       Cliente A              Cliente B
           │                     │
           └──────── TCP ────────┘
                      │
           ┌──────────┴──────────┐
           │   Servidor DRCP     │
           │                     │
    ┌──────┴──────┐          ┌─────────┐
    │   Listener  │          │ Handler │
    │ (main loop) │          │ (thread)│
    └─────────────┘          └────┬────┘
                                  │
                       ┌──────────▼────────┐
                       │ Resource Manager  │
                       │ (sincronizado)    │
                       └───────────────────┘
```

---

## Componentes: Parser e Resource Manager

### Parser (parser.cpp/hpp)

- Interpreta mensagens DRCP
- Extrai comando e parâmetros
- Retorna estrutura ParseResult

### Resource Manager (resource_man.cpp/hpp)

- Gerencia pool de recursos
- Controla estado de reserva
- Usa semáforos para sincronização
- Máximo de 40 recursos

---

## Componentes: Handler e Listener

### Handler (handler.cpp/hpp)

- Processa uma requisição por cliente
- Chama Resource Manager
- Formata respostas DRCP
- Executa em thread dedicada

### Listener (main.cpp)

- Socket servidor TCP
- Aceita conexões
- Cria thread por cliente
- Loop principal

---

## Recursos do Sistema

### Estrutura de Recurso

```c
typedef struct {
  int id;              // Identificador único
  char* value;         // Valor (string/int)
  bool reserved;       // Status de reserva
  pthread_t* belongs_to; // Thread proprietária
} Resource;
```

### Características

- **ID**: Identificador único auto-incrementado
- **Valor**: Armazenável como string ou inteiro
- **Estado**: Livre ou Reservado
- **Proprietário**: Thread cliente que o reservou
- **Limite**: Máximo de 5 recursos simultâneos

---

## Requisitos Funcionais (Parte 1)

### 1. CREATE(valor)

- **Input**: Valor (string/int)
- **Output**: ID do novo recurso
- **Erro**: Limite atingido (5)

### 2. GET(id)

- **Input**: ID do recurso
- **Output**: Valor do recurso
- **Erro**: Não reservado ou inexistente

### 3. SET(id, valor)

- **Input**: ID e novo valor
- **Output**: OK
- **Erro**: Não reservado ou inexistente

---

## Requisitos Funcionais (Parte 2)

### 4. RESERVE(id)

- **Input**: ID do recurso
- **Output**: OK
- **Erro**: Já reservado ou inexistente

### 5. RELEASE(id)

- **Input**: ID do recurso
- **Output**: OK
- **Erro**: Não reservado por você

### 6. LIST()

- **Input**: Nenhum
- **Output**: Contagem + Lista
- **Erro**: Nenhum

---

## Formato de Requisições

**Estrutura: COMANDO [PARÂMETROS]**

### Exemplos

```
CREATE "Meu recurso"
GET 10
SET 10 "Novo valor"
RESERVE 5
RELEASE 5
LIST
```

Simples! Uma linha por requisição.

---

## Formato de Respostas

### Sucesso

```
👍 [RESULTADO]
```

Exemplos: `👍 10` | `👍 "valor"` | `👍`

### Erro ❌

```
😡 [CÓDIGO] [DESCRIÇÃO]
```

Exemplos: `😡 2 Recurso inexistente` | `😡 1 Já reservado`

---

## Estrutura de Dados - Response

```c
typedef struct {
  int status_code;  // OK ou ERROR
  int id;           // Se aplicável
  char* value;      // Se aplicável
  int count;        // Para LIST
  Resource** list;  // Para LIST
} Response;
```

---

## Códigos de Erro

| Código | Significado           | Casos de Uso                                     |
| ------ | --------------------- | ------------------------------------------------ |
| **1**  | Recurso já reservado  | GET, SET, RESERVE em recurso reservado por outro |
| **2**  | Recurso inexistente   | GET, SET, RESERVE, RELEASE de ID inválido        |
| **3**  | Recurso não reservado | Tentativa de GET/SET/RELEASE sem reservar        |
| **5**  | Limite de recursos    | CREATE com 5 recursos já existentes              |
| **7**  | Método inexistente    | Comando não reconhecido pelo parser              |

---

## Fluxos com Erros (Parte 1)

### Cenário 1: Acesso sem Reserva

```
Cliente A: GET 10
Servidor:  😡 3 Não reservado
```

### Cenário 2: Recurso Já Reservado

```
Cliente A: RESERVE 10
Servidor: 👍
Cliente B: GET 10
Servidor: 😡 1 Já reservado
```

---

## Fluxos com Erros (Parte 2)

### Cenário 3: Limite de Recursos

```
Cliente: CREATE R1  → 👍 0
Cliente: CREATE R2  → 👍 1
...
Cliente: CREATE R40  → 😡 5 Limite atingido
```

---

## Sistema de Logging

### Formato: [TIMESTAMP][#ID_CLIENTE][TIPO] [PARÂMETROS]

**Tipo REQUEST:**

- CREATE valor
- GET/SET/RESERVE/RELEASE id [valor]
- LIST

**Tipo ANSWER:**

- `0 👍` ou `0 👍 resultado`

**Tipo CONNECT/DISCONNECT:**

- Sem parâmetros

---

## Exemplo de Log Real

```
[2026-04-13 11:23:17][#00011][CONNECT]
[2026-04-13 11:23:17][#00011][REQUEST] CREATE Rec_Cliente_1
[2026-04-13 11:23:17][#00011][ANSWER] 0 👍 1
[2026-04-13 11:23:17][#00011][REQUEST] RESERVE 1
[2026-04-13 11:23:17][#00011][ANSWER] 0 👍
[2026-04-13 11:23:17][#00011][REQUEST] GET 1
[2026-04-13 11:23:17][#00011][ANSWER] 0 👍 Rec_Cliente_1
[2026-04-13 11:23:17][#00011][REQUEST] LIST
[2026-04-13 11:23:17][#00011][ANSWER] 0 👍 2 0 1
[2026-04-13 11:23:17][#00011][DISCONNECT]
```

---

## Sincronização e Concorrência

### Mecanismos

- Semáforos (POSIX) para proteção crítica
- Mutex para acesso aos recursos
- Variáveis de thread para proprietário

### Operação Thread-Safe

1. Lock semáforo
2. Acessar/modificar
3. Unlock semáforo
4. Responder cliente

### Cleanup Automático

```c
void release_all_from_client(pthread_t* client_id);
```

Libera recursos ao desconectar
