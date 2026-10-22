<!-- translated from docs/getting-started.md, sha256 db71c88bd2a7 -->
# Primeiros passos

Do cartão SD a um console configurado para a sua criança, em cerca de dez minutos, e depois as tarefas do dia a dia. Volte ao [README](../../README.md).

## Antes de começar

- **Um console rodando o Atmosphère.** O PlayGuard comanda o controle parental do próprio console por meio de um serviço restrito: ele precisa de um firmware personalizado. O console não é desbloqueado? O PlayGuard não pode ajudar ([solução de problemas](troubleshooting.md#console-não-desbloqueado)).
- **Um firmware compatível.** Do 21.0.0 ao 23.0.1, todos os recursos; nos mais antigos, tudo menos o temporizador; um mais recente abre somente leitura até uma versão oferecer suporte a ele ([compatibilidade](compatibility.md)).
- **Um único limite para o console inteiro.** O temporizador é do console: ele é o mesmo para todos os usuários. Limites diferentes para cada criança exigem outra ferramenta ([por quê](compatibility.md#playguard-ou-um-sysmodule-substituto)).

Quer dar uma olhada antes? [Experimente o PlayGuard no navegador](https://playguard.link/simulator/?not_set_up=1), com um console simulado na primeira abertura.

## 1. Instale

Extraia o `playguard.zip` da [versão mais recente](https://github.com/JigSawFr/PlayGuard/releases/latest) na raiz do cartão SD ([instalação](install.md)). O PlayGuard ainda não está na Homebrew App Store nem no sphaira: a ficha dele ainda aguarda aprovação.

> [!TIP]
> Extraia também o `playguard-rescue.zip`, o sysmodule de recuperação opcional. Ele não faz nada até você precisar dele, e é o único jeito de voltar a ter acesso se um limite de 0 minuto bloquear tudo e o PIN tiver sido esquecido ([Console bloqueado?](troubleshooting.md#console-bloqueado)).

## 2. Abra do jeito certo

Abra o PlayGuard **por cima de um jogo** (segure R ao iniciar um jogo, com o hbmenu, o sphaira ou a Homebrew App Store), com **o usuário de um responsável**:

- por cima de um jogo, o PlayGuard roda como aplicativo, com toda a memória dele; pelo álbum, ele roda como applet, com menos;
- o console conta o tempo passado no PlayGuard como tempo desse jogo, e o atribui ao usuário escolhido ao abrir: o de um responsável, não o de uma criança.

## 3. Siga os *Primeiros passos*

Na primeira abertura, o guia *Primeiros passos* aparece (depois: *Sobre › Ajuda e diagnóstico › Primeiros passos*). *Hoje* mostra as mesmas etapas em um cartão até elas serem concluídas (*1 de 3*):

1. **O PIN.** O PIN do próprio console, pedido pelo menu HOME para iniciar um jogo além do limite ou para mudar as configurações. Evite 0000, 1234 ou 2580: o PlayGuard avisa sobre um PIN que qualquer pessoa tentaria primeiro.
2. **Um limite diário.** O mesmo todos os dias, ou um para os dias úteis e outro para o fim de semana. Mude qualquer dia isolado depois, em *Tempo de jogo*.
3. **O relógio de rede.** O temporizador conta com ele. Um console que nunca acessa os servidores da Nintendo o mantém impreciso: veja a etapa 5.

## 4. Desvincule o app do celular

O console ainda está vinculado ao app *Controle parental* da Nintendo (ou é um console usado vinculado ao de outra pessoa)? *PIN › Desvincular o app do celular*. Senão, a próxima sincronização do app substitui o que você definiu no PlayGuard. Funciona offline.

## 5. Ajuste o relógio de rede

*Console › Relógio de rede › Medir a diferença de horário* consulta três servidores de horário públicos; depois, *Ajustar o relógio de rede*. Ajustar o relógio recomeça o tempo de jogo de hoje, então faça isso antes de a criança jogar.

Um console sem internet: quando nenhum servidor responde, o PlayGuard oferece ajustar o relógio de rede pelo relógio do console. Confira primeiro o relógio do console com um celular ou um relógio, com *Configurações do console › Console › Data e hora › Sincronizar relógio pela Internet* desativado ([detalhes](features.md#relógio-de-rede)).

## 6. Escolha o que acontece quando o tempo acaba

*Tempo de jogo › Quando o tempo acabar*: o console mostra "Tempo esgotado" e depois **suspende o jogo** ou **só avisa**. Suspender interrompe o jogo sem briga, mas o progresso não salvo pode ser perdido; avisar deixa a decisão com a criança. Acrescente um **alarme de hora de dormir** na mesma aba, se quiser.

## 7. Proteja o próprio PlayGuard

*PIN › Proteger o PlayGuard com o PIN*:

- **Antes de alterar** (o padrão) — qualquer pessoa pode olhar, só o responsável muda algo;
- **Para abrir o PlayGuard** — uma tela de bloqueio na abertura;
- **Nunca** — o PlayGuard avisa isso no sino enquanto houver um PIN definido.

## 8. Entregue o console

*Hoje › Mostrar à criança* mostra só o tempo restante e quando o jogo termina, grande o bastante para ser lido do outro lado da sala. Ⓐ e o PIN trazem de volta as telas do responsável.

> [!IMPORTANT]
> **Um jogo deixado aberto no menu HOME continua contando.** Feche-o, ou coloque o console no modo de espera ([o que conta](features.md#o-que-conta-como-tempo-de-jogo)).

## Tarefas do dia a dia

| Para… | Onde |
|---|---|
| Dar mais tempo hoje | *Hoje › Tempo extra* (+15 min, +30 min, +1 h). Isso se desfaz sozinho no dia seguinte. |
| Dar mais tempo em outro dia | *Hoje › Tempo extra › Outro dia…* |
| Terminar por hoje | *Hoje › Terminar por hoje*: agora, ou daqui a 5 minutos para salvar. |
| Mudar o limite de um dia | *Tempo de jogo*: Ⓐ no dia, ↑/↓, depois **+** para salvar. |
| Alternar entre período de aulas e férias | *Tempo de jogo › Perfis*; *Perfil agendado* para um período escolhido com antecedência. |
| Ver quem jogou o quê | *Atividade*: por jogo, por conta, hoje, 7 dias, desde sempre; exportação ou um QR code. |
| Bloquear todos os jogos por enquanto | *PIN › Bloquear todos os jogos*. Ao desativar, os limites anteriores voltam. |
| Desfazer uma mudança | **ZR** *Desfazer* logo depois dela, ou *Preferências › Histórico de alterações* a qualquer momento. |
| Guardar uma cópia das configurações | *Preferências › Fazer backup das configurações* (nunca o PIN). |
| Saber o que precisa de você | O sino (Ⓨ): o que fazer em âmbar, o que saber com um "i". |

## Próximos passos

- Cada aba e cada configuração: [recursos](features.md).
- Algo não funciona como esperado: [solução de problemas](troubleshooting.md#problemas-comuns).
- O que o console faz, e com que certeza sabemos disso: [como o controle parental funciona](../parental-controls.md) (em inglês).
