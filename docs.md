# Protocolo Distributed Resource Control Protocol (DRCP)

### Métodos

- CREATE(valor string/int) -> id int | error
  - Caso sucesso: id_do_recurso_criado int
  - Casos erro:
    - Limite de recursos (5)
    - Valor inválido (6)

- GET(id int) -> valor string/int
  - Caso sucesso: valor string/int
  - Casos de erro:
    - Recurso já reservado por alguém (1)
    - Recurso inexistente (2)
    - Recurso livre porém não reservado (3)

- SET(id int, valor string/int) -> ok
  - Caso sucesso: ok
  - Casos de erro:
    - Recurso já reservado por alguém (1)
    - Recurso inexistente (2)
    - Recurso livre porém não reservado (3)
    - Valor inválido (6)

- RESERVE(id int)
  - Caso sucesso: ok
  - Casos erro:
    - Recurso já reservado por alguém (1)
    - Recurso inexistente (2)

- RELEASE(id int)
  - Caso sucesso: ok
  - Casos erro:
    - Recurso já reservado por alguém (1)
    - Recurso inexistente (2)
    - Recurso livre porém não reservado (3)

Recurso struct {
id int
reservado bool
pertence bool
}

- LIST()
  - Caso sucesso: count int, lista_de_ids (Recurso)[]
  - Casos de erro: nenhum

Request ->
1 linha -> método
2 linha -> parâmetros separados por (new line)

Response ->
1 linha -> sucesso (indicado por um joinha) ou erro (indicado por um X)
caso sucesso -> resto é resposta
caso erro ->

X
404
Não encontrado

### Códigos de erro

- Recurso já reservado (1)
- Recurso inexistente (2)
- Recurso não reservado (3) (Release um recurso não reservado)
- Recurso não é teu (4) !deprecated
- Limite de recursos (5)
- Valor inválido (6)
- Método inexistente (7)
- Bad request (8)
- Internal server error (500)

### Exemplos

Cliente:
CREATE
23
Meu recurso muito legal

Servidor:
👍
10

Cliente:
GET
10

Servidor:
😡​
3
Recurso livre porém não reservado

Cliente:
RESERVE
10

Servidor:
👍

Cliente:
GET
10

Servidor:
👍
23
Meu recurso muito legal

### Logging

Operações de protocolo

CREATE valor cliente
GET valor cliente
SET id valor cliente
RESERVE id cliente
RELEASE id cliente
LIST cliente

Erros

ERRO codigo descricao

Conexões e desconexões

CONEXAO cliente
DESCONEXAO cliente

### Tarefas

- Gerenciador de recursos
  int create()
  string get()
  ...

- Adapter
  void handler(request DRCPRequest) {

}

- Parser / Serializer
  DRCPRequest parse(bytes)

- Listener
  accept and create threads
