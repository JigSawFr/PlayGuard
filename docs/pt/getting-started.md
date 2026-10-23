<!-- translated from docs/getting-started.md, sha256 e84c84f4627e -->
# Como começar

Do cartão SD a uma consola configurada para o seu filho, em cerca de dez minutos, e depois as tarefas do dia a dia. Voltar ao [README](../../README.md).

## Antes de começar

- **Uma consola com o Atmosphère.** O PlayGuard comanda o controlo parental da própria consola através de um serviço restrito: precisa de um firmware personalizado. A consola não está modificada? O PlayGuard não pode ajudar ([resolução de problemas](troubleshooting.md#consola-não-modificada)).
- **Um firmware suportado.** Da 21.0.0 à 23.0.1, todas as funcionalidades; nos anteriores, tudo menos o temporizador de jogo; num mais recente, abre só de leitura até uma versão o suportar ([compatibilidade](compatibility.md)).
- **Um só limite para toda a consola.** O temporizador de jogo é o da consola: é o mesmo para todos os utilizadores. Limites diferentes para cada criança precisam de outra ferramenta ([porquê](compatibility.md#playguard-ou-um-sysmodule-de-substituição)).

Quer dar uma vista de olhos primeiro? [Experimente o PlayGuard no navegador](https://playguard.link/simulator/?not_set_up=1), com uma consola simulada no seu primeiro arranque.

## 1. Instale-o

Extraia o `playguard.zip` da [versão mais recente](https://github.com/JigSawFr/PlayGuard/releases/latest) para a raiz do cartão SD ([instalação](install.md)). O PlayGuard ainda não está na Homebrew App Store nem no sphaira: a sua ficha ainda aguarda aprovação.

> [!TIP]
> Extraia também o `playguard-rescue.zip`, o sysmodule de recuperação opcional. Não faz nada até precisar dele, e é a única forma de voltar a entrar se um limite de 0 minutos bloquear tudo e o PIN tiver sido esquecido ([Consola bloqueada?](troubleshooting.md#consola-bloqueada)).

## 2. Abra-o da forma certa

Abra o PlayGuard **sobre um jogo** (mantenha R premido ao iniciar um jogo, com o hbmenu, o sphaira ou a Homebrew App Store), com **o utilizador de um adulto**:

- sobre um jogo, o PlayGuard corre como aplicação, com toda a sua memória; a partir do álbum, corre como applet, com menos;
- a consola conta o tempo passado no PlayGuard como tempo desse jogo, e atribui-o ao utilizador escolhido ao iniciar: o de um adulto, não o de uma criança.

## 3. Siga os *Primeiros passos*

No primeiro arranque, aparece o guia *Primeiros passos* (mais tarde: *Preferências › Primeiros passos*). *Hoje* mostra os mesmos passos num cartão até estarem feitos (*1 de 3*):

1. **O PIN.** O PIN da própria consola, pedido pelo menu HOME para iniciar um jogo depois do limite ou para alterar as definições. Evite 0000, 1234 ou 2580: o PlayGuard assinala um PIN que qualquer pessoa experimentaria primeiro.
2. **Um limite diário.** O mesmo todos os dias, ou um para os dias úteis e outro para o fim de semana. Mais tarde, altere qualquer dia em separado em *Tempo de jogo*.
3. **O relógio de rede.** O temporizador de jogo conta com ele. Uma consola que nunca se liga aos servidores da Nintendo mantém-no impreciso: veja o passo 5.

## 4. Desvincule a aplicação móvel

Ainda associada à aplicação *Controlo Parental* da Nintendo (ou uma consola em segunda mão associada à de outra pessoa)? *Segurança › Desvincular a aplicação móvel*. Caso contrário, a próxima sincronização da aplicação substitui o que definiu no PlayGuard. Funciona offline.

## 5. Acerte o relógio de rede

*Consola › Relógio de rede › Medir a diferença horária* consulta três servidores de hora públicos; depois, *Acertar o relógio de rede*. Acertar o relógio repõe a zero o tempo de jogo de hoje, por isso faça-o antes de a criança jogar.

Uma consola sem Internet: quando nenhum servidor responde, o PlayGuard propõe acertar o relógio de rede pelo relógio da consola. Confira primeiro o relógio da consola com um telemóvel ou um relógio, com *Definições da consola › Consola › Data e hora › Sincronizar relógio através da Internet* desativado ([detalhes](features.md#relógio-de-rede)).

## 6. Escolha o que acontece quando o tempo acaba

*Tempo de jogo › Quando o tempo acabar*: a consola mostra "Tempo esgotado" e depois **suspende o jogo** ou **apenas avisa**. Suspender para o jogo sem batalhas, mas o progresso não guardado pode perder-se; avisar deixa a decisão à criança. Acrescente um **alarme de hora de dormir** no mesmo separador, se quiser.

## 7. Proteja o próprio PlayGuard

*Segurança › Proteger o PlayGuard com o PIN*:

- **Antes de uma alteração** (a predefinição) — qualquer pessoa pode ver, só o pai ou a mãe altera alguma coisa;
- **Para abrir o PlayGuard** — um ecrã de bloqueio ao arrancar;
- **Nunca** — o PlayGuard assinala-o no sino enquanto houver um PIN definido.

## 8. Entregue a consola

*Hoje › Mostrar à criança* mostra só o tempo restante e a hora a que o jogo termina, em tamanho suficiente para se ler do outro lado da sala. Ⓐ e o PIN trazem de volta os ecrãs do adulto.

> [!IMPORTANT]
> **Um jogo deixado aberto no menu HOME continua a contar.** Feche-o, ou ponha a consola em modo de suspensão ([o que conta](features.md#o-que-conta-como-tempo-de-jogo)).

## Tarefas do dia a dia

| Para… | Onde |
|---|---|
| Dar mais tempo hoje | *Hoje › Tempo extra* (+15 min, +30 min, +1 h). É retirado sozinho no dia seguinte. |
| Dar mais tempo noutro dia | *Hoje › Tempo extra › Outro dia…* |
| Terminar por hoje | *Hoje › Terminar por hoje*: agora, ou daqui a 5 minutos para guardar. |
| Alterar o limite de um dia | *Tempo de jogo*: Ⓐ no dia, ↑/↓, depois **+** para guardar. |
| Alternar entre o tempo de aulas e as férias | *Tempo de jogo › Perfis*; *Perfil agendado* para um período escolhido com antecedência. |
| Ver quem jogou a quê | *Atividade*: por jogo, por conta, hoje, 7 dias, desde sempre; uma exportação ou um código QR. |
| Bloquear todos os jogos por agora | *Tempo de jogo › Bloquear todos os jogos*. Quando é desativado, os limites anteriores voltam. |
| Anular uma alteração | **ZR** *Anular* logo a seguir, ou *Histórico › Alterações do PlayGuard* a qualquer momento. |
| Guardar uma cópia das definições | *Histórico › Criar uma cópia de segurança* (nunca o PIN). |
| Saber o que precisa de si | O sino (Ⓨ): o que fazer a âmbar, o que saber com um "i". |

## A seguir

- Todos os separadores e definições: [funcionalidades](features.md).
- Algo não funciona como esperado: [resolução de problemas](troubleshooting.md#problemas-comuns).
- O que a consola faz, e com que certeza o sabemos: [como funciona o controlo parental](../parental-controls.md) (em inglês).
