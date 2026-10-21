<!-- translated from docs/features.md, sha256 733c8f08dff8 -->
# Recursos

O que cada aba faz, como o PlayGuard grava as configurações com segurança e o que o console conta como tempo de jogo. Novo no PlayGuard? Comece pelos [primeiros passos](getting-started.md). Volte ao [README](../../README.md).

<!-- github-only: the website shows each language's screenshots, in the page's theme -->

As capturas de tela mostram o tema escuro, o padrão do PlayGuard (em inglês).

| Hoje | Console |
|---|---|
| ![Hoje](../../images/screenshots/dashboard.png) | ![Console](../../images/screenshots/console.png) |
| **Tempo de jogo** | **Tempo de jogo › editando um dia** |
| ![Tempo de jogo](../../images/screenshots/play_timer.png) | ![Editando um dia](../../images/screenshots/per_day.png) |
| **Atividade** | **Um jogo** |
| ![Atividade](../../images/screenshots/activity.png) | ![Um jogo](../../images/screenshots/activity_game.png) |
| **Restrições** | **Console › Relógio de rede** |
| ![Restrições](../../images/screenshots/restrictions.png) | ![Relógio de rede](../../images/screenshots/clock.png) |
| **PIN** | **Preferências** |
| ![PIN](../../images/screenshots/security.png) | ![Preferências](../../images/screenshots/preferences.png) |

## Controles

| Botão | O que faz |
|---|---|
| ↑/↓ | Mover |
| Ⓐ | Confirmar |
| Ⓑ | Voltar ou cancelar (duas vezes na barra lateral para sair) |
| Ⓧ | Atualizar (nas abas que leem o console) |
| Ⓨ | Notificações, em qualquer aba |
| **−** | Idioma e tema, em qualquer aba (o rodapé não os mostra; o sino, o idioma e o tema do cabeçalho mostram) |
| **+** | Salva os limites por dia (e a semana editada em *Tempo de jogo*) |
| **ZR** | *Desfazer*, por alguns segundos depois que uma mudança é salva |

O app é organizado em abas, como as Configurações do console; cada uma tem sua seção abaixo.

- **O que falhou é dito em uma caixa de diálogo, o que deu certo em uma mensagem rápida.** Uma caixa de diálogo só diz o que vale para este console agora: que o jogo em andamento seria suspenso, só quando há um jogo aberto; que o app do celular poderia substituir uma mudança, só enquanto ele estiver vinculado e puder sincronizar.
- **O cabeçalho** indica, em todas as abas, quando o controle parental está desbloqueado temporariamente (a etiqueta dele), e mostra um lápis riscado à direita do título quando o app está somente leitura; um toque em qualquer um deles leva aonde isso é desfeito (*PIN › Reativar o controle dos pais*, a tela de firmware).
- **No modo somente leitura**, as ações continuam no lugar, em cinza, e dizem por quê quando pressionadas.

## O visual: Cadran

O visual do PlayGuard se chama Cadran: uma barra de ícones para as abas, uma paleta azul-marinho ou branco frio (conforme o tema), cartões em vez de listas. Cada aba abre no que mais importa (um anel, blocos, cartões, botões), com as outras configurações embaixo.

- **A etiqueta do cabeçalho** diz do que trata a aba (*Controle dos pais ativo*, *Temporizador ativo*…); enquanto o controle parental estiver desbloqueado, ela avisa isso em todas as abas, e um toque nela abre *PIN › Reativar o controle dos pais*. O texto secundário nunca fica abaixo de 15 px, para ser lido do sofá em uma TV.
- **As caixas de diálogo** são cartões com botões em forma de pílula, a confirmação em verde-azulado (em coral quando ela remove ou interrompe algo).
  - As escolhas abrem em um cartão acima das indicações dos botões, a atual sobre um verde-azulado suave com a marca de seleção. O contorno verde-azulado é só do foco, em todo lugar: um nível, uma conta, uma escolha em vigor mantém o fundo suave.
  - Uma caixa de diálogo longa vai esmaecendo na parte de baixo enquanto há mais conteúdo, e o aviso dela (um jogo prestes a ser suspenso) vem primeiro, em um cartão âmbar.
  - Uma falha tem como título o que falhou, o código de erro pequeno abaixo do motivo, e oferece *Enviar um relatório* quando ajuda.
  - As mensagens aparecem como cartões abaixo do cabeçalho, com uma marca de seleção verde-azulada para o que foi feito e uma marca âmbar para o que foi recusado.
- **As outras telas** ganham o mesmo visual: o editor por dia de um perfil mostra a semana como em *Tempo de jogo* (editada do mesmo jeito, com os preenchimentos rápidos em blocos); *Verificar os jogos instalados* e *Todos os jogos* colocam as linhas em cartões; *Perfis* e *Ajuda e diagnóstico* dão a cada linha seu ícone.
- **Tamanho do texto** (*Preferências*, ou **AA** no cabeçalho):
  - *Automático*, o padrão, aumenta todos os textos em 25 % enquanto o console está na base, na TV, e volta ao normal no modo portátil; *Maior* mantém o texto maior, *Normal* nunca.
  - O cabeçalho, a barra lateral e o rodapé mantêm o tamanho (não há espaço para mais), assim como os nomes dos níveis de Restrições, que já são grandes.
  - A mudança é imediata, em todas as telas, sem reiniciar.
  - Os 25 % vêm de uma TV de 55" em 4K: o menor texto (15 px) tem então 14,6 mm de altura em maiúsculas, 20′ de arco a 2,5 m, onde a leitura é confortável (16′ no tamanho normal, o mínimo indispensável).
- **Efeitos visuais** (*Preferências*): *Mais leves com bateria*, o padrão, deixa de fora o halo de fundo, a pulsação do anel da tela da criança nos últimos minutos e o deslizar do arco até um novo valor enquanto nenhum carregador estiver conectado (modo portátil, fora da tomada); *Sempre completos* mantém tudo. A configuração é lida de novo a cada 5 segundos, então ligar o console na tomada traz os efeitos de volta.
- **Desfazer:** depois que uma mudança é salva, **ZR** *Desfazer* fica no rodapé por alguns segundos: ele abre essa mudança no histórico de alterações, para restaurar o valor anterior. Um histórico de alterações vazio avisa isso em um cartão.

## Notificações — o que precisa de você, em qualquer aba

Um sino no cabeçalho, à esquerda da versão, reúne o que precisa do responsável; Ⓨ abre a lista em qualquer aba (o contador do sino mostra *2* quando há duas coisas a fazer).

À esquerda do sino, o idioma (o código dele, *FR*, *EN*…) e o tema (um sol para claro, uma lua para escuro, um círculo meio preenchido quando segue o console), depois o tamanho do texto (**AA**, com o A grande em verde-azulado enquanto o texto estiver maior): um toque abre a mesma lista de *Preferências*, e **−** oferece essas opções em qualquer aba. Um novo tema ou idioma vale na hora, com cada tela refeita na mesma aba e o foco na mesma linha, chinês e sua própria fonte incluídos.

### A fazer

Cada um com um sinal de alerta âmbar, contado no sino em âmbar; fica até ser resolvido:

- ainda sem PIN, ou um PIN que qualquer pessoa tentaria primeiro (0000, 1234, 1212, 2580…: avaliado no console, nunca mostrado);
- um PIN no console enquanto o próprio PlayGuard abre sem ele (*Proteger o PlayGuard com o PIN* em *Nunca*);
- o controle parental desbloqueado temporariamente (*Reativar*);
- tempo extra de um dia anterior ainda a desfazer;
- o alarme "Tempo esgotado" desativado;
- mudanças feitas fora do PlayGuard;
- o console sem contar o tempo de jogo;
- um relógio de rede impreciso.

### Para informação

Cada um com um "i", um ponto no sino; pode ser marcado como lido:

- um limite ainda reduzido por *Encerrar o jogo por hoje* em um dia anterior (*Restaurar*);
- uma nova versão instalada (*Novidades*; depois de uma atualização no próprio app, a compilação anterior guardada), ou uma versão mais nova do PlayGuard;
- um relatório enviado online (*Mostrar o link*), ou o PlayGuard fechado inesperadamente da última vez (*Enviar um relatório*);
- o tempo de jogo da semana (a cada 7 dias até ser lido; com a comparação, por dia, com a semana anterior; *Mostrar o QR code* abre o resumo da semana em *Atividade*), e um limite ultrapassado em dois ou mais dos últimos 7 dias enquanto o console só avisa;
- um backup das configurações a fazer (nenhum ainda, ou nenhum há 30 dias);
- todos os jogos bloqueados há uma semana;
- uma nova conta de usuário;
- o app do celular vinculado;
- o relógio de rede encontrado fora do horário na abertura.

### Console

As entradas sobre o console, e não sobre o controle parental, vêm por último, em uma linha que Ⓐ abre (aberta de cara quando não há mais nada). As que são a fazer são contadas no selo da aba Console, não no sino:

- jogos instalados que não conseguem iniciar (*Verificar os jogos instalados*), patches de jogos que precisam de atenção;
- o cartão SD quase cheio (menos de 500 MB);
- um dos arquivos do próprio PlayGuard que não pôde ser posto de lado ou gravado no cartão SD;
- o sysmodule de recuperação desatualizado (não é o que vem com esta versão, ou foi instalado antes de informar a própria versão; *Atualizar* instala o que esta versão traz), depois o console a reiniciar para usá-lo, até ele ser reiniciado;
- uma compilação que não terminou de abrir e da qual o PlayGuard voltou (*Enviar um relatório*).

As para informação: uma volta para a compilação anterior, um firmware não testado com esta versão, um Atmosphère, hekate ou firmware mais novo (verificado com *Procurar atualizações todos os dias*; a entrada do firmware diz para esperar até o Atmosphère oferecer suporte), o número de série não mascarado no emuMMC, o controle parental próprio do outro sistema, um dos arquivos do próprio PlayGuard encontrado danificado e posto de lado (uma mensagem rápida na abertura aponta para ele).

### Como as entradas se comportam

- Cada seção lista suas entradas em uma ordem fixa, seja qual for a verificação que terminou primeiro.
- **Na barra lateral**, um selo âmbar ao lado do nome de uma aba conta as entradas a fazer dela, onde elas são resolvidas: a aba PIN (sem PIN, um PIN fraco, o PlayGuard sem o PIN, desbloqueado temporariamente), Tempo de jogo (o alarme desativado, o console sem contar), *Hoje* (mudanças feitas fora do PlayGuard, tempo extra a desfazer), o Console (as entradas acima, e o relógio de rede). As entradas ignoradas e as para informação não são contadas.
- Na aba, o mesmo selo fica na linha onde cada uma é resolvida (*Relógio de rede*, *Verificar os jogos instalados*, *Armazenamento*, *Patches de jogos*; *Definir ou alterar o PIN*, *Proteger o PlayGuard com o PIN*, *Reativar o controle dos pais*; *Avançado* do Tempo de jogo para o alarme), e a aba abre na primeira delas.
- **Ⓐ** em uma entrada aplica a solução (*Ativar*, *Acertar*, *Atualizar*…) ou abre a aba onde ela é feita. Uma entrada some sozinha quando o que ela diz é resolvido.
- **Ⓧ** marca uma entrada para informação como lida; **Ⓨ** marca todas. Uma entrada marcada como lida continua lida na próxima abertura, até o texto dela mudar.
- **Ignorar:** algumas entradas a fazer podem ser ignoradas com Ⓧ, depois de uma confirmação e do PIN: um PIN que qualquer pessoa tentaria primeiro, o PlayGuard abrindo sem o PIN, o alarme "Tempo esgotado" desativado, o cartão SD quase cheio (nunca a falta de PIN, o controle desbloqueado, o relógio, o temporizador sem contar ou os arquivos do próprio PlayGuard). Uma entrada ignorada deixa de ser contada e aparece por último em *Ignorados*, onde Ⓐ volta a contá-la. Ela volta sozinha 30 dias depois (a menos que *Contar de novo os avisos ignorados após 30 dias* esteja desativado), quando o que ela diz é resolvido e depois acontece de novo, ou, para o PIN, com um novo PIN de outro tamanho.
- *Preferências › Notificações* desativa três tipos: versões do Atmosphère, do hekate e do firmware; o tempo de jogo da semana; lembretes (backups, o outro sistema, todos os jogos bloqueados).
- O estado por trás disso é lido quando o PlayGuard abre, a cada minuto na tela principal e a cada 5 s enquanto *Hoje* está na tela.

## Hoje — o dia em um relance

*[Experimente no simulador ›](https://playguard.link/simulator/)*

### O anel e a linha

**O tempo restante** em um anel: um dia sem jogo mostra *Sem jogar*, não *0 min*; o tempo extra dado hoje aparece como um segmento mais claro no final, e o anel desliza até cada novo valor.

Abaixo dele, o que vai interromper o jogo primeiro:

- antes do fim do tempo ou do alarme de hora de dormir, o resto do dia como uma linha, com o momento atual à esquerda, cada um deles uma marca com o horário (*16:40 tempo esgotado*, *21:00 hora de dormir*), em verde-azulado até o primeiro, em âmbar nos últimos 15 minutos dele;
- senão, em palavras: o alarme de hora de dormir depois que o tempo do dia se esgota, quando o jogo volta a ser permitido; enquanto o controle parental estiver desbloqueado, o limite de hoje em pausa e por quê.

### As ações

- **Tempo extra**, com os valores a um toque: um valor selecionado move na linha o fim do tempo (*com mais 15 min*), ou avisa na marca do alarme de hora de dormir que ele continua vindo primeiro; o primeiro se destaca em verde-azulado quando restam 15 min ou menos, ou nada.
- **Terminar por hoje**, **Mostrar à criança**.
- **Limite de hoje…**: o limite de hoje escolhido ali mesmo, gravado depois de uma confirmação.
- **O perfil em vigor**, pelo nome, depois que um perfil é salvo (*Perfis…* quando nenhum corresponde): Ⓐ lista os perfis, com o que está em vigor marcado, para aplicar outro depois de uma confirmação (os dias que mudam), ou *Gerenciar os perfis…*.
- Enquanto o controle parental estiver desbloqueado, **Reativar** vem primeiro, ocupando a largura toda da linha, e *Tempo extra* sai de cena.
- Sem limite em nenhum dia, **Definir os limites da semana** abre a semana em *Tempo de jogo*.

### Os cartões e as etiquetas

- O **alarme de hora de dormir** (quando o jogo volta a ser permitido, ou daqui a quanto tempo ele toca quando o console não informa; desativado, *Desativado* em letras grandes e Ⓐ *Definir*), o **controle parental** e o **relógio de rede**, em cartões.
- O estado do console em **etiquetas**:
  - uma diz se o redirecionamento DNS do Atmosphère bloqueia os servidores da Nintendo, e Ⓐ nela diz quais e o que isso muda: sem sincronização para o app do celular, um relógio de rede que não se ajusta mais sozinho;
  - outra, quando aberto por cima de um jogo, que o tempo passado no PlayGuard conta, com Ⓐ explicando como;
  - um jogo deixado aberto no menu HOME aparece como uma etiqueta âmbar (*continua contando*, com Ⓐ explicando por quê), quando a etiqueta sobre o PlayGuard aberto por cima de um jogo ainda não diz isso.
- As etiquetas mostram primeiro o que precisa do responsável (âmbar, com um triângulo), depois o que está ativado (verde-azulado, com uma marca de seleção), depois o resto (cinza, com um ponto): a forma diz isso tanto quanto a cor.
- Abaixo das etiquetas, o tempo extra ainda a desfazer (Ⓐ o desfaz) e o alarme "Tempo esgotado" enquanto estiver desativado (Ⓐ o reativa).

### Primeiros passos

Até o PIN e os limites da semana serem definidos, um cartão acima dos cartões lista as três etapas (o PIN, um limite diário, o relógio de rede), cada uma marcada em verde-azulado quando concluída ou circulada em âmbar, com quantas estão concluídas (*1 de 3*); Ⓐ em uma delas a faz ali mesmo.

### O console está contando?

O tempo do próprio PlayGuard conta enquanto ele roda como aplicativo; então, com um limite hoje, o tempo restante deve diminuir enquanto ele está aberto. Quando ele não muda por cerca de 90 s na tela (temporizador ativado, um limite hoje, sem desbloqueio, não aberto pelo álbum), uma linha âmbar abaixo do tempo jogado diz *O console não está contando o tempo de jogo agora*, com as causas prováveis:

- o relógio foi atrasado (volta a contar mais tarde);
- o relógio de rede nunca foi ajustado (*Console › Relógio de rede*);
- um limite acabou de ser gravado (abra um jogo por um minuto e depois confira).

Ela some assim que o tempo restante volta a mudar. O Tempo de jogo também avisa.

### Mudanças fora do PlayGuard

O tempo de jogo de hoje recomeçando no mesmo dia (uma mudança de relógio faz isso), o relógio do console movido em mais de 5 minutos, limites diferentes dos que o PlayGuard viu por último (o app do celular, outra ferramenta): um cartão âmbar acima do anel até ser dispensado (Ⓐ) ou até o dia seguinte, e uma entrada no histórico de alterações. O PlayGuard não roda em segundo plano: ele só vê o que mudou enquanto estava aberto, ou desde a última vez que esteve, quando lê o console de novo.

### No limite

O console mostra "Tempo esgotado" e depois suspende o jogo (sem "continuar") ou só avisa: *Quando o tempo acabar* mostra qual e permite mudar, depois que um limite ou uma hora de dormir é definido ([detalhes, em inglês](../parental-controls.md#when-the-time-runs-out)).

### Tempo extra e encerrar o jogo por hoje

- **Tempo extra hoje**: +15 min, +30 min, +1 h por padrão.
- **Encerrar o jogo por hoje**: limite 0 só para hoje, *agora* ou *daqui a 5 minutos* quando o tempo jogado é conhecido (o limite passa a ser o tempo jogado + 5 min, tempo para salvar). No bloqueio, o "Tempo esgotado" do console aparece, e o progresso não salvo pode ser perdido — a confirmação avisa.
- **No dia seguinte** — na abertura, ou à meia-noite se o app estiver aberto — o PlayGuard volta ao limite de sempre:
  - depois de tempo extra, sozinho, sem perguntar e sem PIN (ele só reduz o limite, como reativar o controle parental), desbloqueando por um instante se o temporizador estiver contando e bloqueando de novo na hora. Ele espera enquanto não pode gravar (modo somente leitura, um firmware sem suporte, o bloqueio do console ativado) e, uma semana depois no mesmo dia, enquanto o limite menor pararia o jogo em andamento (aí, no dia seguinte). Para ter mais tempo em um dia, mude o limite desse dia;
  - depois de *Encerrar o jogo por hoje*, ele pergunta se mantém 0 ou volta ao limite (com o PIN, como qualquer mudança que aumenta um limite), ou volta ao limite como qualquer mudança (*Tempo de jogo › Voltar ao limite após "Encerrar o jogo por hoje"*); voltar para "sem limite" sempre é perguntado.
- **Tempo extra em outro dia** (*Tempo extra › Outro dia…*, no fim da lista): amanhã ou um dos cinco dias seguintes, cada um listado com seu limite, e depois o valor. O limite desse dia é aumentado na hora, só para essa data, e restaurado sozinho (ele só reduz o limite) na próxima vez que o PlayGuard for aberto depois que o dia terminar — não em um console em que o PlayGuard não é aberto, onde o mesmo dia da semana seguinte também fica com ele. Um de cada vez: a lista então oferece removê-lo (com o limite restaurado na hora). Uma mudança no limite desse dia nesse meio-tempo (a semana, um perfil…) prevalece: nada é restaurado.

### A tela da criança

*Mostrar à criança* em *Hoje*, para entregar o console:

- só o tempo restante (enquanto o controle parental estiver desbloqueado, o limite de hoje, em pausa), quando o jogo termina (*Termina por volta das 16:40*; quando o alarme de hora de dormir vem antes, o anel faz a contagem regressiva até ele, *45 min antes de dormir*, como na última hora de um dia sem limite), a hora de dormir e o que acontece quando o tempo acaba;
- feita para ser lida do outro lado da sala, na TV; o texto dela mantém o tamanho seja qual for o tamanho do texto;
- nos últimos 5 minutos (de jogo ou antes da hora de dormir), o fim fica âmbar e o anel pulsa devagar; a hora de dormir fica âmbar nos últimos 30;
- Ⓐ e Ⓨ, os botões do responsável, aparecem em tamanho pequeno no canto superior direito, para que o rodapé mostre só o da criança;
- Ⓐ volta às telas do responsável com o PIN; Ⓨ dá tempo extra dali mesmo, depois do PIN (os valores de *Tempo extra*, sem sair da tela da criança); Ⓑ sai do PlayGuard (perguntando antes se deve reativar o controle parental, como ao sair).

Com o texto maior, *Hoje* continua cabendo em uma só tela: a linha acima do título só mostra o dia e o limite dele (*QUINTA-FEIRA · 2 h*), e os botões e os cartões ficam um pouco mais apertados. A tela se atualiza a cada 5 s (Ⓧ atualiza agora).

## Tempo de jogo — a semana é o editor

*[Experimente no simulador ›](https://playguard.link/simulator/?tab=play_timer)*

### Editar a semana

Ⓐ em um dia o edita:

| Botão | Na edição |
|---|---|
| ↑/↓ | Muda o limite em 15 min, de hora em hora quando mantidos pressionados |
| ←/→ | Passa para o dia seguinte |
| Ⓛ | Copia o limite para segunda a sexta |
| Ⓡ | Copia o limite para o fim de semana |
| Ⓨ | Sem limite |
| Ⓧ | Uma duração exata (*Inserir uma duração…*: ↑/↓ de 15 em 15 min, de hora em hora quando mantidos pressionados, Ⓛ/Ⓡ de 5 em 5 min, com OK selecionado) |
| Ⓐ ou Ⓑ | Encerra a edição |

- Pelo toque, um dia tocado entra em edição, e depois um toque na parte de cima ou de baixo dele muda o limite em 15 min.
- As mudanças são salvas juntas, com **+** ou *Salvar*, como no editor por dia (a confirmação lista os dias alterados, antes → depois); *Descartar* desfaz as mudanças.
- Dentro da barra de cada dia, o tempo jogado na última ocorrência dele (incluindo hoje, todas as contas), em coral quando o limite foi atingido, e o limite também, e uma legenda abaixo da semana avisa (*limite ultrapassado*).
- Um dia alterado e ainda não salvo tem o limite em âmbar e a barra contornada em âmbar; abaixo da semana, os números do dia selecionado (*Quinta-feira: 1 h 20 jogado de 2 h*).
- Acima da semana, em âmbar, um limite atingido e o que o console faz nesse caso, ou o console sem contar o tempo de jogo.
- `0` minuto significa *sem jogar nesse dia*; *Remover o limite de tempo de jogo* desativa o temporizador.

### Botões rápidos e ajustes rápidos

**Botões rápidos** acima e abaixo da semana abrem as opções na hora: os perfis (quantos estão salvos), o mesmo limite todos os dias, o alarme de hora de dormir, *Quando o tempo acabar* e a remoção.

- **Mesmo limite todos os dias:** uma lista rápida ou uma duração exata (*Inserir uma duração…*: ↑/↓ de 15 em 15 min, de hora em hora quando mantidos pressionados, Ⓛ/Ⓡ de 5 em 5 min).
- **Dias úteis e fim de semana ›:** o editor, pedindo o limite de segunda a sexta e depois o do fim de semana.
- **Tempo extra hoje** e **encerrar o jogo por hoje**, como em *Hoje*. Uma nota abaixo deles diz o que o console faz no limite. Enquanto o controle parental estiver desbloqueado temporariamente, o tempo de jogo *talvez* não seja contado: isso ainda está em aberto ([docs, em inglês](../parental-controls.md#still-open)).
- Cada confirmação lista os dias que mudam, antes → depois.

### Perfis

- **Perfis** salvos no cartão SD: aplicar (um perfil já em vigor só avisa isso), editar, renomear ou excluir um; salvar os limites atuais ou criar um novo.
- Qualquer nome, em qualquer escrita (*École*, *周末*, *Выходные* …) — um nome sem letras latinas recebe um nome de arquivo próprio, e dois nomes que dariam no mesmo arquivo são detectados.
- **Perfil agendado** (*Perfis › Perfil agendado*): um perfil para um período — férias, uma viagem — com o primeiro e o último dia escolhidos entre os próximos 60 dias.
  - Quando o período começa, o PlayGuard pergunta se deve aplicá-lo (com os dias que ele muda desenhados abaixo da pergunta); quando termina, se deve restaurar os limites que ele substituiu, ou mantê-los.
  - A pergunta é feita uma vez por dia, na tela principal, quando não há mais nada na frente: o PlayGuard só age quando é aberto, então um console em que ele não é aberto fica com os limites que tem. Um período que passou sem que o perfil tenha sido aplicado é esquecido.
  - Um de cada vez; a linha o altera, remove ou encerra.

### Alarme de hora de dormir

- O horário do alarme (das 16:00 às 23:45, ou *Desativado*) e quando o jogo volta a ser permitido (das 05:00 às 09:00), o mesmo todos os dias, definidos com o analógico: para cima e para baixo um quarto de hora, L e R uma hora.
- Ou, com Ⓨ *Por dia* na caixa de diálogo do alarme, de segunda a sexta, o fim de semana ou um único dia, com os outros dias mantendo o seu (cada grupo de dias listado com seu horário). Quando os dias são diferentes, a aba mostra *Varia conforme o dia*, e a semana em *Tempo de jogo* mostra o alarme de cada dia abaixo do nome dele.
- A posição dele nas configurações do temporizador, deduzida das configurações do app do celular, foi verificada em um console (22.0.0); mesmo assim, o PlayGuard só o altera quando o console informa o que o PlayGuard lê ali, confere a resposta do console depois da mudança e restaura as configurações anteriores se ela for diferente.

### Configurações de tempo extra e Avançado

- **Tempo extra:** os valores oferecidos (+15/+30/+1 h, +10/+20/+30 min …) e a volta ao limite de sempre sem perguntar depois de *Encerrar o jogo por hoje* (o tempo extra sempre volta sem perguntar). As duas coisas são feitas na próxima vez que o PlayGuard é aberto, a partir do dia seguinte: um console em que ele não é aberto fica com o limite daquele dia.
- **Avançado ›**, uma tela própria: a chave das ações de depuração e, enquanto ela estiver ativada, o alarme "Tempo esgotado" ativado/desativado e pausar / retomar a contagem. A mesma chave permite que um backup ou o histórico restaure o alarme.

## Atividade — quem jogou o quê, e por quanto tempo

*[Experimente no simulador ›](https://playguard.link/simulator/?tab=activity)*

Tempo por jogo **hoje**, nos **últimos 7 dias** e **desde sempre**, do próprio registro de atividade do console — para todas as contas ou **uma conta de usuário** (as contas em botões no topo, quando o console tem várias). O PlayGuard abre na última conta mostrada, ou na definida em *Conta padrão* (todas as contas, ou uma).

### No topo

- As contas em botões, o período e a exportação.
- Hoje, os últimos 7 dias e desde sempre em números. Abaixo dos últimos 7 dias, a comparação, por dia, com os 7 anteriores: ▲ *+20 min/dia vs semana anterior* em âmbar, ▼ menos em verde-azulado, *como na semana anterior* com até 5 minutos de diferença.
- Os quatro jogos mais jogados em cartões (Ⓐ abre um); nos últimos 7 dias, aquele cujo tempo mais aumentou em relação à semana anterior diz quanto, a partir de meia hora (▲ *+2 h*).
- Nada jogado no período: um cartão avisa, com *Ver desde sempre* quando há algo para ver.

### Gráficos e resumo

- **Um gráfico dos últimos sete dias** (hoje à direita, uma legenda embaixo), com o limite desta semana para cada dia como uma linha e o tempo acima dele em âmbar, marcado com !; os totais de hoje, da semana e de sempre.
- **Um resumo** do período escolhido (hoje, os últimos 7 dias, os **últimos 30 dias** ou desde sempre), para a conta mostrada: média por dia, o jogo mais jogado, os dias jogados e o mais movimentado (últimos 7 dias), a sessão média (desde sempre).
- **Horas de jogo, últimos 7 dias**: um quadrado para cada hora de cada dia, começando pela segunda-feira, mais forte quanto mais se jogou (todos os jogos, a conta mostrada); depois as sessões (jogar com pausas de menos de 5 minutos conta como uma única sessão, mesmo trocando de jogo) e a duração média delas, a mais longa e o dia dela, as duas horas seguidas mais jogadas, e o tempo jogado entre 21:00 e 06:00.

### Todos os jogos e a página de um jogo

- **Todos os jogos**, em uma tela própria: cada jogo jogado no período, os mais jogados primeiro, como **grade de capas** ou **lista** (Ⓨ alterna, e a escolha é mantida), com os números do jogo selecionado sempre no topo: ícone e nome, o tempo de jogo no período, desde sempre, as inicializações e a última vez que foi jogado. Ⓧ escolhe o período (a aba Atividade acompanha).
- Os ícones são lidos em torno do jogo selecionado (não no modo applet, para poupar memória); um controle cinza fica no lugar até o ícone chegar, e para um jogo que não tem ícone.
- Uma biblioteca grande mostra os primeiros 50 jogos, depois *Mostrar todos os jogos* (no máximo 100 jogos no modo applet); a exportação sempre contém todos. O PlayGuard lê no máximo 256 jogos (os desta semana primeiro): além disso, a aba Atividade, Todos os jogos e a exportação (menos o CSV) avisam que alguns estão faltando, e o total desde sempre os deixa de fora.
- **Ⓐ em um jogo:** os últimos sete dias dele em barras, inicializações, a primeira e a última vez que foi jogado, o tempo por conta de usuário. Os jogos excluídos mantêm os números de sempre, mesmo um excluído antes de o PlayGuard ser instalado, desde que o registro de jogo do console ainda o mencione, e o nome, depois que o PlayGuard o viu (senão, *Jogo excluído* e o ID do título).

### Contas

- O tempo de um jogo que nenhuma conta de usuário tem (alguns jogos nunca pedem uma, ou a conta foi excluída depois) aparece como *Sem conta*, a partir de um minuto.
- Uma conta de usuário que o PlayGuard nunca viu antes é marcada em âmbar na aba por 30 dias (*Conta nova desde …*: criar uma conta nova é um jeito conhecido de recomeçar do zero) e registrada no histórico de alterações; as contas que existem quando o PlayGuard as lista pela primeira vez não são marcadas.

### Exportação e compartilhamento

- **Exportação para o cartão SD** (o botão *Exportar*) em CSV, JSON, XLSX (Excel) ou PDF, uma coluna por dia, e depois os últimos 7 dias, os últimos 30 dias e desde sempre.
- **O resumo da semana em um QR code:** a câmera de um celular o lê como algumas linhas de texto, no idioma mostrado — os últimos 7 dias e a média deles, a tendência, cada dia, os três jogos mais jogados, as sessões, a mais longa e o tempo jogado entre 21:00 e 06:00. Nada vai para a internet.
- Abre na hora com os últimos números lidos (guardados no cartão SD entre uma execução e outra), atualizados em segundo plano quando têm mais de um minuto; Ⓧ os lê de novo agora, com um indicador de carregamento.
- Os tempos são aproximados se o relógio do console foi alterado.

## Restrições — nível, classificação indicativa, comunicação

*[Experimente no simulador ›](https://playguard.link/simulator/?tab=restrictions)*

- **O nível de restrição:** Criança pequena, Criança, Adolescente e Personalizado lado a lado em blocos, cada um com o que restringe (um cadeado para o que é restrito, uma marca de seleção para o que é permitido, e os jogos instalados que ele bloqueia; Ⓐ o escolhe; com *Personalizado* em vigor, Ⓐ leva às configurações dele, mais abaixo); *Nenhum* abaixo deles. O que um nível restringe (limite de idade, publicações, comunicação) é dito de novo antes de ele ser aplicado.
- **Em Personalizado:** classificação indicativa, publicação em redes sociais, comunicação com outras pessoas.
- **Modo VR** e o **sistema de classificação** (PEGI, ESRB, USK, CERO …): a lista diz para quais países cada um classifica (PEGI: a maior parte da Europa, ESRB: Estados Unidos, Canadá e América Latina, USK: Alemanha, PEGI BBFC: Reino Unido…).
- **O número de jogos com permissão para se comunicar** (a lista em si está no relatório de diagnóstico, em formato bruto, até a estrutura dela ser conhecida).
- **Jogos instalados bloqueados:** quantos dos jogos instalados o nível em vigor bloqueia pela classificação indicativa (*3 de 15*), com Ⓐ listando-os com a classificação (*PEGI 16*); o bloco de cada nível também mostra isso (*Jogos bloqueados · 3*).
  - As classificações são as de cada jogo (os dados de controle dele, lidos pela verificação dos jogos instalados na abertura ou em *Console › Verificar os jogos instalados*), pelo sistema que o console usa.
  - Um jogo que esse sistema não classifica não é contado, nem as exceções definidas no app do celular.
  - A tela de um jogo em Atividade mostra a classificação dele, e *bloqueado no nível atual* quando for o caso.

## PIN — o PIN, o desbloqueio, o bloqueio dos jogos, o app do celular

*[Experimente no simulador ›](https://playguard.link/simulator/?tab=security)*

- **No topo**, três blocos: o PIN, as restrições e o desbloqueio temporário, cada um com uma marca de seleção verde-azulada quando está como deveria (um PIN definido, restrições ativas, bloqueado) ou um sinal de alerta âmbar quando é preciso agir.
- **Definir ou alterar o PIN** (tela de PIN do sistema).
- **Mostrar o PIN**: depois de um aviso e do próprio PIN, pedido toda vez; registrado no histórico, e depois é oferecido um novo PIN.
- **Desbloquear temporariamente**, **reativar o controle dos pais**, e se o PlayGuard o reativa sozinho logo depois de uma mudança que precisou do desbloqueio (ativado por padrão).

### Proteger o PlayGuard com o PIN

O PIN no próprio PlayGuard:

| Configuração | Efeito |
|---|---|
| *Nunca* | Nenhum PIN no PlayGuard (o sino avisa enquanto o console tiver um) |
| *Antes de alterar* (padrão) | Qualquer pessoa pode olhar, só o responsável muda algo |
| *Para abrir o PlayGuard* | Uma tela de bloqueio na abertura |

- Depois de digitado, o PIN vale até o PlayGuard fechar; *Pedir o PIN de novo após 5 minutos* (desativado por padrão) pede de novo depois desse tempo, e traz de volta a tela de bloqueio de *Para abrir o PlayGuard* quando o PlayGuard volta depois de 5 min ou mais fora de foco.
- Verificado na camada de serviço, então nenhuma mudança escapa; bloquear de novo nunca pede o PIN. *Mostrar o PIN* sempre o pede, em todos os modos e mesmo logo depois de ele ter sido digitado, e um `config.json` ausente ou danificado conta como *Antes de alterar*.
- **Sair** enquanto o controle parental ainda está desbloqueado pergunta se deve bloqueá-lo de novo (*Reativar e sair* também fecha o jogo por cima do qual o PlayGuard foi aberto, voltando ao menu HOME em vez do hbmenu); se ele for fechado pelo menu HOME, o PlayGuard o bloqueia de novo na próxima abertura, e reiniciar o console o bloqueia sozinho.

### Bloquear todos os jogos

Uma chave que define o limite de todos os dias como 0, então é preciso um PIN para iniciar um jogo — um bloqueio leve, sem classificação indicativa nem limites de comunicação.

- Ele bloqueia o início dos jogos, não o menu HOME, e precisa de um PIN.
- Os limites anteriores voltam quando ele é desativado.
- Enquanto ele estiver ativado, *tempo extra* e *encerrar o jogo por hoje* são recusados, e limites definidos de outro jeito (um perfil, um backup, o histórico…) o substituem.

### App do celular

- Se o app Controle parental do Nintendo Switch está vinculado, a última sincronização dele, e **desvincular** (senão, a próxima sincronização dele substitui os limites definidos aqui: enquanto ele estiver vinculado, cada mudança que o PlayGuard pede para confirmar avisa isso).
- Quando o redirecionamento DNS do Atmosphère (`atmosphere/hosts`) desvia o servidor de controle parental da Nintendo, o app não consegue sincronizar: o PlayGuard então mostra *sincronização bloqueada (DNS)* aqui e não menciona o app em nenhum outro lugar (nenhuma nota nas caixas de diálogo, nenhuma entrada no sino, nenhuma etapa nos primeiros passos, nenhuma etiqueta em *Hoje*, nenhuma ajuda para decifrar, e desvincular pergunta sem dizer o que o app substituiria).
- Enquanto ele estiver vinculado e puder sincronizar, *Antes de desvincular: decifre…* abre a comparação do bloco (veja [CONTRIBUTING.md](../../CONTRIBUTING.md#no-code-needed), em inglês).

### Excluir todo o controle dos pais

Duas confirmações, irreversível; um backup das configurações é salvo antes.

## Console — o relógio de rede, os jogos instalados, o que o console executa

*[Experimente no simulador ›](https://playguard.link/simulator/?tab=console)*

**No topo**, o que o console executa em blocos (o firmware, o Atmosphère, e o hekate quando está no cartão SD; Ⓐ abre as notas de versão dele), depois o diagnóstico, depois as linhas do console, começando pela compatibilidade (só uma marca de seleção verde-azulada quando o PlayGuard foi verificado neste firmware, senão um alerta e o que está errado).

### Relógio de rede

*Relógio de rede ›*, uma tela própria — o relógio de que o temporizador depende. Um console que nunca acessa os servidores da Nintendo mantém esse relógio impreciso, o que distorce o temporizador.

- **Relógios do console e de rede**, fuso horário, precisão. Quando o redirecionamento DNS do Atmosphère (`atmosphere/hosts`) desvia o servidor de horário da Nintendo (`aauth-lp1.ndas.srv.nintendo.net`), o console não consegue mais ajustar o relógio de rede sozinho: esta tela, a entrada do sino e o cartão de *Hoje* avisam isso e apontam para a medição abaixo.
- **Escolha de um servidor NTP público** (≈ 50 incluídos, por região, ou o seu).
- **Medir** com 3 servidores (mediana; um aviso âmbar quando eles discordam) e depois **ajustar o relógio de rede** — uma medição continua válida por 2 minutos, com uma contagem regressiva. A confirmação avisa: mudar o relógio recomeça o tempo de jogo de hoje, então o limite inteiro fica disponível de novo.
- **Uma verificação do relógio de rede na abertura**: uma mensagem rápida quando ele está mais de um minuto fora; ela nunca ajusta o relógio.

#### Nenhum servidor responde

Em um console sem internet, *Ajustar o relógio de rede pelo relógio do console* aparece abaixo do resultado (e no cartão do relógio de rede de *Hoje*, depois que a medição dele falha), atrás da mesma confirmação.

- O PlayGuard não tem mais nada com que comparar o relógio do console: a confirmação mostra o horário dele e pede que você o confira primeiro com um celular ou um relógio, e um relógio do console anterior a 2026 (nunca ajustado) é recusado.
- Com *Sincronizar relógio pela Internet* desativado, o relógio do console fica onde está, e ativar a configuração depois não o faz mais pular; com ela ativada, o relógio do console já segue o relógio de rede, então a confirmação diz para desativá-la e acertar o horário à mão se ele estiver errado.
- Só o relógio é gravado, não o controle parental, então não é preciso desbloquear (o PIN de *Proteger o PlayGuard com o PIN* continua valendo); como qualquer mudança de relógio, isso recomeça o tempo de jogo de hoje.
- Ainda não testado em um console.

### Jogos instalados

*Verificar os jogos instalados* encontra os jogos que o menu HOME não consegue iniciar ou mostrar — um ícone que fica carregando, um "!", uma moldura pontilhada — com o que está errado e o que fazer, abaixo do ícone do jogo, e quantos jogos instalados têm um problema.

**O que ela encontra:** nome e ícone ilegíveis, o próprio jogo faltando por trás da atualização, nada mais instalado, arquivos faltando, dados corrompidos encontrados pelo console, um firmware mais novo exigido pelo jogo ou só pela atualização dele, chaves novas demais para o firmware, uma licença (ticket) faltando, o console pedindo uma atualização que foi removida, o cartão SD ilegível, uma inicialização recusada pelo console.

**A lista tem uma seção por saída**, cada uma com sua contagem e o que fazer, os jogos com o mesmo problema juntos (dois jogos com o mesmo nome são diferenciados pela versão ou pelo ID do título):

| Seção | O que contém |
|---|---|
| **Reparáveis aqui** | O que o PlayGuard repara |
| **A verificar** | Dados de que o console suspeita: a verificação diz se é preciso instalar o jogo de novo |
| **Reinstalar** | Arquivos sumidos ou inutilizáveis, ou dados encontrados corrompidos: remover e depois instalar de novo |
| **Firmware ou cartão SD** | O jogo está bem, o console não |

- **Verificar os dados:** o que a verificação de dados encontrou fica com o jogo até outra versão ser instalada: corrompidos, ele passa para *Reinstalar* como *Danificado (verificado)*, com a data e o código do console; intactos, ele sai da lista. *Verificar estes jogos* verifica toda a seção *A verificar*, um jogo depois do outro em uma única caixa de diálogo (*jogo 2 de 5*), e depois diz quantos estão corrompidos, intactos ou não puderam ser verificados, com a remoção dos corrompidos à mão.
- **Vários jogos de uma vez:** Ⓨ seleciona jogos (uma marca de seleção na frente), e *Vários jogos de uma vez*, acima das seções, seleciona uma seção inteira; depois, uma ação para todos: verificar os dados ou removê-los (a lista deles salva antes, uma pergunta, um PIN).
- **Em *Reinstalar*,** a lista pode ser salva (JSON e texto, na pasta de exportações: nome, ID do título, versão, problema), e todos podem ser removidos de uma vez depois de uma pergunta e um PIN: a lista é salva antes, e depois os jogos são removidos para que um instalador não os conte mais como instalados.
- **As ações:** quando uma ação é a solução, ela é o primeiro botão da caixa de diálogo; as outras vêm em seguida: aceitar de novo a versão instalada (quando o console pede uma atualização removida), remover só a atualização ou um conteúdo adicional, verificar se os dados estão corrompidos (a verificação do próprio console, com o progresso e Cancelar) ou remover o jogo.
- As versões aparecem como o jogo as mostra (*versão 1.4.0*, *atualização nº 4*); depois de um reparo, uma mensagem rápida diz se o jogo voltou a funcionar. Os dados salvos sempre ficam; cada mudança está no histórico de alterações.
- **Quando ela verifica:** a última verificação aparece na hora; depois, só os jogos cuja entrada mudou são verificados de novo (todos os jogos uma vez por dia, depois de uma atualização do firmware, ou com Ⓧ *Verificar de novo*), cada um mostrado assim que é verificado.
- Vários jogos com problema ao mesmo tempo apontam para o cartão SD ou para um jogo instalado pelo outro armazenamento (sysMMC / emuMMC).
- *Visualização*, acima das seções, mostra os jogos como **lista** ou como **capas** (ícone, nome e problema; a escolha é mantida), como em *Todos os jogos*.

### Forwarders

*Forwarders ›*, uma tela própria em *Homebrew* — um homebrew no menu HOME, com o ícone e o nome dele, aberto dali como um jogo (um forwarder de NRO, como os que o sphaira cria).

- **Um cartão no topo** diz como está o menu homebrew: bloqueado ou aberto, os caminhos de volta dele (o forwarder do PlayGuard, os menus protegidos, os que ainda não foram abertos pelo menu HOME), o que ainda o abre sem o PIN e se o módulo de resgate está instalado. Ⓐ nele lista o que fazer, cada coisa a um toque (o próximo passo até o bloqueio, proteger ou excluir um forwarder que o abre sem o PIN, a verificação, a folha de resgate).
- **A lista por grupo:** o do próprio PlayGuard, os protegidos com o PIN, os sem o PIN, os que outro app criou, os que não estão mais no menu HOME. Um protegido tem um cadeado no ícone (um visto para um menu que já foi visto funcionando pelo portão dele); um criado aqui sem o PIN, um cadeado aberto em âmbar.
- **Ⓨ** abre esta parte do guia (um QR code). Depois de excluir ou esquecer um forwarder, ou de mudar os horários sem PIN dele, *Pedir o PIN* ou *Fechado quando o tempo de jogo de hoje acaba*, **ZR** o restaura por alguns segundos (instalado novamente com o mesmo ID do programa, ou a configuração como estava).
- **PlayGuard** com um toque só: o próprio ícone e nome, com o forwarder abrindo o `.nro` pelo qual o PlayGuard foi aberto.
- **Outro homebrew**: qualquer `.nro` da pasta `/switch` do cartão SD (e uma pasta abaixo), com o nome pedido antes (o do próprio `.nro`, para manter ou mudar), depois confirmado com o que o menu HOME vai mostrar: o ícone, o nome e a editora dele. O ícone é o do `.nro`, convertido no JPEG de 256 × 256 que o menu HOME mostra quando ele tem outro tamanho ou é um PNG (um controle no lugar dele quando o `.nro` não tem nenhum), como o sphaira faz. Um caminho com espaço é passado ao homebrew entre aspas.
- **Os forwarders do menu HOME**, cada um com o seu ícone: os que o PlayGuard criou (Ⓐ: detalhes, mudar o nome, mudar a editora, instalar novamente, excluir) e os que outro app criou (detalhes, excluir). Ⓐ abre uma folha em seções (*Este forwarder*, *Proteção*, *Instalar e excluir*) sob o ícone e o estado dele, cada linha com o seu ícone e o que está configurado agora (os horários; um interruptor para *Pedir o PIN* e *Fechado quando o tempo de jogo de hoje acaba*), *Excluir* em vermelho; B ou um toque fora dela a fecha. *Detalhes* lista as regras dele do mesmo jeito, e depois a editora, a versão, o `.nro` e o ID do programa; Ⓐ numa regra (ou na editora) a altera, como na folha. Embaixo dos protegidos, uma linha diz o que cada ícone que aparece ali significa. Um cujo `.nro` sumiu avisa isso; um excluído nas Configurações do console continua na lista até ser instalado novamente ou esquecido. Instalar novamente mantém o ID do programa, então a entrada do menu HOME é substituída, não duplicada; um forwarder criado por outra ferramenta para o mesmo `.nro` também é substituído.
- **Protegido com o PIN** (Ⓐ › *Proteger com o PIN*, para um que o PlayGuard criou, fora o dele próprio): aberto pelo menu HOME, o forwarder abre antes um pequeno portão (`config/playguard/gate.nro`, algumas centenas de kilobytes que o PlayGuard grava a partir dos próprios arquivos), que mostra na hora a tela do PIN do console e depois o homebrew quando o PIN está certo; B ali volta ao menu HOME. Em volta da tela do PIN, a do próprio portão: o ícone e o nome do homebrew, *Protected by PlayGuard*, e como foi (iniciando, no horário livre, PIN não digitado), no idioma do console, no azul-marinho do Cadran (`make gate-preview` a desenha em um computador). Ele mantém o ID do programa e o ícone, e o histórico de alterações registra isso. A linha dele mostra as regras em ícones, um para cada regra ativada: uma chave para o PIN, um cadeado aberto para horários sem PIN, um quadrado (stop) para horários em que nunca abre, um timer para fechado quando o tempo de jogo de hoje acaba; *Detalhes* (Ⓐ) as diz por extenso. É preciso um PIN no console. *Parar de pedir o PIN* pede o PIN antes, seja qual for a configuração de *Proteger o PlayGuard com o PIN*. Ele protege só o ícone do menu HOME: o `.nro` continua abrindo pelo menu homebrew, a não ser que ele esteja bloqueado (abaixo). Um forwarder protegido por um PlayGuard anterior abre o próprio PlayGuard como portão, o que continua funcionando; *Reinstalar todos* o passa para o pequeno portão.
  - **Horários sem PIN** (Ⓐ em um protegido): todos os dias, de segunda a sexta ou no fim de semana, de um horário a outro (passando da meia-noite também), ou *De segunda a sexta, e o fim de semana separado* (um intervalo para cada); *Detalhes* diz quando. Defini-los pede o PIN.
  - **Horas em que nunca abre** (Ⓐ em um protegido), definidas do mesmo jeito: aí o portão diz *Não neste horário* e os horários, e volta ao menu HOME, mesmo com o PIN digitado (a hora de dormir dos homebrews). Elas vêm antes dos horários sem PIN.
  - **Fechado quando o tempo de jogo de hoje acabar** (Ⓐ em um protegido): quando o temporizador do console não tem mais tempo hoje, o portão diz *O tempo de jogo de hoje acabou* e volta ao menu HOME, com ou sem PIN. Desativado, a não ser que seja definido.
  - **As regras ficam no forwarder, não no cartão SD:** os horários, o limite do dia e a pausa após recusas vão para os dados do próprio forwarder (no armazenamento do console), então um computador não consegue acrescentar horários livres editando um arquivo; mudar um deles refaz o forwarder, em poucos segundos. O forwarder também guarda o SHA-256 do portão (`config/playguard/gate-<hash>.nro`): um portão alterado no cartão SD não é aberto (erro 2347-0100, [solução de problemas](troubleshooting.md#um-forwarder-protegido-mostra-o-erro-2347-0100)). Um protegido por um PlayGuard anterior lê as regras no cartão: os horários sem PIN dele não são respeitados então, e o sino oferece *Refazer*.
  - **Uma pausa após PIN recusados** (*Pausa após PIN recusados*, em *Menu homebrew*): cinco recusas seguidas (B na tela do PIN) e o forwarder não pede mais o PIN por 15 minutos, com o portão dizendo até quando; 3, 5 ou 10 tentativas e 15 minutos ou uma hora, ou nunca. O sino diz qual forwarder está em pausa, e a linha dele também (um ícone de pausa, e até quando); *Tirar a pausa* (Ⓐ nele, com o PIN pedido) o faz pedir de novo na hora. Cada pausa e cada pausa tirada ficam no histórico de alterações.
  - **Cada tentativa fica no histórico de alterações**: aberto com o PIN, aberto no horário dele, ou recusado (B na tela do PIN), no horário em que aconteceu; as tentativas de um mesmo tipo, em um mesmo dia, para um mesmo forwarder formam uma só entrada, com os horários delas. O portão as anota no cartão SD; o PlayGuard as lê na próxima abertura. *Histórico › Mostrar › Homebrew e forwarders* lista só essas, os forwarders e o bloqueio.
  - **Tentativas da semana** (em *Histórico* e em *Menu homebrew*): nos últimos sete dias, as aberturas de cada forwarder, as recusas dele e em quais dias, e as vezes em que ele não abriu; guardadas por quatro semanas. Quando as recusas desta semana passam do dobro das da semana anterior (cinco no mínimo), o sino avisa.
  - **A tela do portão diz mais:** os horários sem PIN dele enquanto pede o PIN, e as recusas do dia até então (em âmbar) enquanto pede e depois de uma recusa, abaixo de *Peça a um responsável para digitar o PIN*. Depois de *Protected by PlayGuard*, um ícone para cada regra que ele carrega, como a linha dele no PlayGuard: um cadeado aberto (horários sem PIN), um quadrado (horários em que ele nunca abre), um temporizador (o tempo de jogo de hoje).
- **O ícone dele a partir do álbum** (Ⓐ › *Mudar o ícone*): uma captura de tela do álbum (as 30 mais recentes, por data), com o quadrado do meio convertido no JPEG de 256 × 256 do menu HOME; ou de novo o do próprio `.nro`. Instalar novamente o mantém.
- **Ícones e idiomas:** um ícone de outro tamanho, um PNG, ou um JPEG com dados EXIF ou codificação progressiva (o menu HOME mostra esses como um "?") vira um JPEG de 256 × 256 simples. Os dados de controle do forwarder só declaram o inglês americano, o idioma do seu único ícone: assim, o ícone aparece em um console configurado em qualquer idioma.
- **Proteger todos os forwarders com o PIN** e **Reinstalar todos** (o ícone e a versão de um `.nro` novo, o portão como este PlayGuard o traz, um excluído do menu HOME de volta): com um toque cada.
- **Bloquear o menu homebrew** (*Menu homebrew*, por último na tela): nem o álbum nem um jogo aberto segurando R abrem mais o menu homebrew, então um homebrew só abre pelo forwarder dele, que pode pedir o PIN.
  - **Passo a passo:** enquanto ele não pode ser bloqueado, Ⓐ lista os passos, numerados, o próximo em âmbar e a um toque (*Próximo passo*): um PIN, os patches de assinatura, o forwarder do PlayGuard, um menu homebrew protegido com o PIN (o sphaira encontrado em `/switch`, senão o hbmenu, criado já protegido de uma vez), aberto uma vez pelo menu HOME; a lista volta depois de cada passo que cria um forwarder.
  - São precisos dois caminhos de volta ao PlayGuard pelo menu HOME: o próprio forwarder dele e um forwarder de um menu homebrew (sphaira ou hbmenu, pelo nome do `.nro`) protegido com o PIN, que deixa todo homebrew, uma loja de homebrew inclusive, atrás do PIN. E também um PIN no console e os patches de assinatura de que os forwarders precisam. Até lá, Ⓐ diz o que falta.
  - O caminho de volta pelo menu só conta depois de visto funcionando: aberto uma vez pelo menu HOME, com o PIN digitado no portão dele (o registro do portão diz isso, lido quando o PlayGuard abre ou bloqueia). Um menu protegido e nunca aberto assim aparece como tal.
  - Bloquear mostra as verificações em uma lista (um visto verde-azulado para o que está pronto, um triângulo âmbar ou coral para o que não está) e pede Ⓐ segurado por 2 segundos em *Bloquear* (ou um dedo segurado nele), com uma barra enchendo enquanto isso; soltando antes, nada muda. Depois vem a **folha de resgate** (gravada também em `config/playguard/rescue.html`, uma página para abrir e imprimir em um computador, no idioma da interface, com o QR code): como desfazer o bloqueio por um computador, ou com o módulo de resgate quando ele está instalado, e um QR code para a ajuda ([abaixo](troubleshooting.md#menu-homebrew-bloqueado)); *Verificar o menu homebrew* também a tem.
  - No emuMMC (ou em um sysMMC com um emuMMC configurado), ele avisa que o bloqueio fica no cartão SD, então os dois sistemas o leem, enquanto os forwarders são de cada sistema: o outro sistema não tem nenhum desses caminhos de volta.
  - Enquanto ele está bloqueado, excluir o forwarder do próprio PlayGuard, ou o último menu protegido (ou tirar o PIN dele), é recusado: desbloqueie primeiro. Excluir um de vários menus protegidos pede confirmação, em coral, citando os que restam.
  - Antes de bloquear, ele oferece proteger de uma vez os forwarders que o PlayGuard criou e que ainda não estão protegidos, e diz quais ainda abririam homebrew sem o PIN (um menu não protegido, ou um que outro app criou), quais gravam no cartão SD (um gerenciador de arquivos, um instalador como o Cyberfoil ou o Goldleaf, um servidor FTP: eles poderiam mudar o arquivo do bloqueio), e quando o uLaunch está instalado (ele abre homebrew do jeito dele, que o bloqueio não alcança).
  - O PIN recusado três vezes ou mais em um mesmo dia no portão de um forwarder é avisado no sino (qual forwarder, o dia, quantas vezes).
  - Quando gravar um arquivo falha, o erro diz por quê quando dá para saber: o cartão SD cheio, ou nada gravado nele (a trava dele, ou o bit de arquivo que as *Tools* do hekate corrigem).
  - *Verificar o menu homebrew* mostra o que o Atmosphère vai ler (se o álbum e um jogo aberto com R abrem o menu homebrew), os caminhos de volta, os patches de assinatura, a versão do Atmosphère em que ele foi bloqueado e o módulo de resgate; o relatório de diagnóstico traz o mesmo.
  - São algumas linhas no fim de `atmosphere/config/override_config.ini`, entre dois comentários do PlayGuard; o resto do arquivo fica como está, e guardado como era em `config/playguard/override_config.ini.backup`. O Atmosphère lê o arquivo a cada abertura de um programa: sem reiniciar. Desbloquear tira essas linhas e grava de volta os valores do próprio Atmosphère para as chaves que o resto do arquivo não define (o Atmosphère mantém o que leu até o console reiniciar).
  - Bloquear e desbloquear pedem o PIN, seja qual for a configuração de *Proteger o PlayGuard com o PIN*, e ficam no histórico de alterações. Enquanto ele está bloqueado, o PlayGuard verifica a cada abertura se os dois caminhos de volta continuam lá; o sino avisa quando um sumiu. Um bloqueio tirado do arquivo fora do PlayGuard (por um computador, por um homebrew) é avisado no sino e no histórico de alterações; o PlayGuard nunca o recoloca sozinho, já que excluir o arquivo é o caminho de volta por um computador. Excluir um forwarder (Configurações do console › Gerenciamento de dados, onde nenhum PIN é pedido) nunca desfaz o bloqueio: só tira um caminho de volta.
  - Por um computador: exclua `atmosphere/config/override_config.ini` (ou recoloque o backup) e depois reinicie o console.
  - Com o módulo de resgate instalado (opcional, [sysmodule/README.md](../../sysmodule/README.md), em inglês), um pedido `RESCUE` também o desfaz na inicialização, e o mesmo acontece com uma atualização do Atmosphère desde o bloqueio (os patches de assinatura dele podem demorar, e nenhum forwarder abre sem eles): a próxima abertura do PlayGuard avisa isso e oferece bloquear de novo.
- **Um forwarder excluído fora do PlayGuard** (nas Configurações do console): avisado uma vez no histórico de alterações, e no sino enquanto durar, com *Instalar novamente* ou *Esquecer* na linha dele.
- **Patches de assinatura:** o console só abre um forwarder com patches de assinatura do fs e do loader (sys-patch, ou arquivos de sigpatch): sem eles, a tela avisa isso acima da lista (*Console › Patches de jogos* tem os detalhes). O `.nro` fica onde está; excluir um forwarder nunca mexe nele.
- **Tempo de jogo:** aberto pelo ícone, um homebrew roda como um jogo: o tempo de jogo o conta e o suspende como qualquer jogo quando o tempo acaba, e *Atividade* o lista. O PlayGuard continua à mão pelo menu de homebrew.
- **Nos backups das configurações** (*Histórico › Backups*): o que o PlayGuard criou e se o menu homebrew estava bloqueado. Restaurar um backup oferece, depois do resto, recolocar os forwarders que não estão mais no menu HOME (protegidos como estavam, com os horários e o ícone deles); um bloqueio só é avisado, para ser feito de novo por aqui.
- Cada alteração pede o PIN quando *Proteger o PlayGuard com o PIN* pede, é recusada no modo somente leitura e fica no histórico de alterações. O que o PlayGuard criou é guardado por sistema (sysMMC / emuMMC), em `forwarders.json`.

### As linhas do console

- **Firmware, Atmosphère e hekate** (quando o `bootloader/update.bin` dele está no cartão SD), cada um com a versão mais nova encontrada pela última busca de atualizações, em âmbar (*22.0.0 · 23.0.1 disponível*, e quantas versões atrás quando é mais de uma), e um ícone de aviso na linha do firmware quando o firmware mais recente precisa antes de um Atmosphère ou hekate mais novo, ou ainda não tem suporte deles (as notas de versão do firmware dizem por quê: as notas de versão deles dizem quais firmwares eles suportam).
- **Notas de versão:** Ⓐ em cada um as abre: uma aba para a versão instalada, uma para cada versão mais nova (no máximo cinco) e versões mais antigas nas abas que sobram (seis no total; Ⓛ / Ⓡ passam de uma para a outra), abrindo na seguinte, com etiquetas (instalada ou mais nova, o firmware mais recente que ela suporta, correções de segurança, controle parental alterado), as notas como publicadas (em inglês), *Ver online* (um QR code) e quando foram baixadas. As do Atmosphère e do hekate vêm das versões deles no GitHub, as do firmware do switchbrew; elas ficam guardadas no cartão SD para leitura offline e são baixadas de novo uma vez por dia (Ⓧ agora).
- **Compatibilidade**, depois **armazenamento** (emuMMC ou sysMMC, com um ícone de informação quando o outro sistema tem seu próprio controle parental: Ⓐ explica).
- Se o Atmosphère **mascara o número de série** (parcialmente oculto até Ⓐ; no emuMMC, quando ele não é mascarado, um ícone de aviso na linha acima, que Ⓐ explica).
- **Patches de jogos** (sys-patch ou arquivos de sigpatch, recomendando o sys-patch quando só arquivos são usados: um ícone de informação, ou de aviso quando algo precisa de atenção, que Ⓐ explica).

## Preferências

*[Experimente no simulador ›](https://playguard.link/simulator/?tab=preferences)*

- O que configura o próprio PlayGuard:
  - Idioma (todos os idiomas do console, ou o do próprio console), tema (escuro por padrão, claro, ou o do console), tamanho do texto (automático / normal / maior): os três na hora.
  - A aba inicial.
  - **A introdução** na abertura: a tela de carregamento por 2 segundos (ativada por padrão; Ⓐ ou Ⓑ pula; desativada, o PlayGuard abre assim que estiver pronto).
- As mensagens que ele mostra:
  - Os tipos de notificações (veja [acima](#como-as-entradas-se-comportam)).
  - Um **lembrete mensal para apoiar o PlayGuard** (ativado por padrão, nunca no primeiro mês nem logo depois de uma atualização; *Não mostrar de novo* no lembrete ou esta chave o desativa de vez, inclusive nas atualizações).
- Uma configuração que molda um recurso fica junto dele: o tempo extra em *Tempo de jogo*, a reativação do controle parental em *PIN*, a verificação do relógio em *Console › Relógio de rede*. As atualizações e *Ajuda e diagnóstico* ficam em *Sobre*.

### Alterações e backups

No fim de *Preferências* (a aba Histórico das versões anteriores).

#### Histórico de alterações

- As 500 alterações mais recentes feitas pelo PlayGuard (limites, nível de restrição, PIN, desbloqueios, desvinculação, o relógio, restaurações, pausar ou retomar a contagem …) e o que ele viu mudar fora dele, agrupadas por dia (*Hoje*, *Ontem*, depois o dia e a data), com o horário e de onde veio; um limite alterado para um único dia cita esse dia.
- Uma alteração **recusada** pelo console também aparece, em âmbar, com o erro do console e o que foi tentado (nunca restaurada: nada mudou).
- *Mostrar* mantém um tipo (tempo de jogo, restrições, segurança e app, visto fora do PlayGuard, recusado pelo console); *Exportar o que é mostrado* grava em `exports/` como CSV, JSON, XLSX ou PDF.
- Ⓐ em uma alteração a mostra e, para um valor, **restaura o anterior** — com o mesmo desbloqueio e o mesmo PIN de qualquer mudança, avisando se ele mudou desde então.
- Cada alteração guarda a versão do PlayGuard que a fez; uma alteração registrada por uma versão mais nova do PlayGuard aparece como *Outra alteração*.

#### Backups

- **Fazer backup / restaurar as configurações** no cartão SD: nível de restrição, configurações personalizadas, modo VR, sistema de classificação, limites diários, o alarme "Tempo esgotado" (com as ações avançadas ativadas), e o bloco bruto do temporizador para registro — nunca o PIN.
- Uma restauração lista só o que mudaria (nada a mudar: ela só avisa, sem desbloqueio e sem gravar nada), e avisa quando o backup tinha um alarme de hora de dormir ativado (ele não é regravado).
- Escolha quantos backups manter.

## Sobre

*[Experimente no simulador ›](https://playguard.link/simulator/?tab=about)*

### Versão e atualizações

- O PlayGuard instalado: versão (em âmbar quando sai uma mais nova; Ⓐ abre as **notas de versão** dela), modo de abertura e pasta de dados.
- **Procurar atualizações** agora; *Procurar atualizações todos os dias* na abertura.
- *Atualizar com*: no PlayGuard, que baixa a versão, verifica, se substitui e reinicia, ou sphaira, Homebrew App Store ou manualmente (perguntado na primeira atualização quando nada foi escolhido). Instalado pela Homebrew App Store ou pela App Store do sphaira, a loja é o caminho recomendado ali, para ela acompanhar a versão; quando o catálogo dela ainda não tem a versão nova (ela publica uma depois de uma revisão, muitas vezes alguns dias depois), o PlayGuard avisa isso e oferece atualizar no próprio lugar.
- Enquanto houver uma versão mais nova conhecida, o PlayGuard abre em **O PlayGuard X.Y.Z está disponível** (a data e as notas dela): *Atualizar*, ou *Mais tarde* por 1 dia, 1 semana ou no máximo 30 dias.
- As notas de versão são as do próprio PlayGuard, na mesma tela que as do Atmosphère e do hekate em *Console*: das versões dele no GitHub, uma aba por versão (a instalada, as mais novas, as mais antigas), cada uma com os destaques em cartões e todas as mudanças, guardadas no cartão SD para leitura offline (as da versão instalada também estão no app, para quando nada foi baixado ainda).
- Depois de uma atualização, o PlayGuard abre uma vez em **Novidades da X.Y.Z** (os mesmos destaques, depois os QR codes).

### Ajuda e diagnóstico

*Ajuda e diagnóstico ›*, uma tela própria, abaixo de *Atualizar com*:

- **Primeiros passos** abre o guia de novo (com uma etapa *Desvincular o app do celular* enquanto ele estiver vinculado, uma etapa *Reativar o alarme "Tempo esgotado"* enquanto ele estiver desativado, uma etapa *Proteger o PlayGuard* depois que um PIN for definido — a escolha de *Proteger o PlayGuard com o PIN* —, e uma chave para ele não aparecer mais na abertura). Abaixo de *Fechar*, *Apoiar o PlayGuard* mostra os QR codes de apoio.
- **Exportar relatório de diagnóstico**, ou **enviar um online** (veja [Relatar um bug](troubleshooting.md#relatar-um-bug)); quantos relatórios salvos, arquivos de falha, logs de sessão e links de relatórios enviados online são mantidos, por quantidade, idade ou tamanho.

### Créditos

Os créditos, como **apoiar o PlayGuard** ([GitHub Sponsors](https://github.com/sponsors/JigSawFr), [Ko-fi](https://ko-fi.com/jigsawfr), mostrados como QR codes para escanear com um celular), e um pequeno *Made in France* 🇫🇷.

## Seguro por design

**Gravar o limite de tempo de jogo com segurança.** Sobrescrever a configuração do temporizador enquanto ele faz a contagem desestabiliza o Atmosphère. Por isso, o PlayGuard verifica o estado primeiro; se o temporizador estiver ativo, a confirmação diz que o controle parental será desbloqueado temporariamente — com o PIN armazenado, **você não precisa lembrá-lo**. Um toque então desbloqueia, confere se o sistema realmente informa o desbloqueio, grava e **bloqueia de novo na hora** (ou oferece fazer isso, se *Reativar o controle dos pais após alterar* estiver desativado). A camada de serviço confere o estado de novo logo antes de gravar, então nenhuma tela consegue pular essa proteção.

> [!CAUTION]
> Um limite menor que o tempo já jogado hoje suspende o jogo assim que o controle parental é bloqueado de novo. A confirmação avisa quando isso vai acontecer.

**Fim das falhas no 22.5.** O `pctl:a`, o serviço privilegiado de controle parental, aceita **uma única sessão**. As compilações antigas do app original a mantinham aberta, então a tela de PIN do menu HOME (ou o applet de PIN) não conseguia obtê-la e o Atmosphère podia falhar. O PlayGuard abre a sessão para cada ação e a libera na hora; as atualizações periódicas pausam enquanto o app está em segundo plano. *(Diagnóstico do [fork do anbingxi](https://github.com/anbingxi/NX-Pctl-Manager/tree/diag/fw22-5-readonly).)*

**Nada roda em segundo plano.** O limite, o PIN, os avisos e a suspensão são do próprio console; o PlayGuard só muda as configurações deles.

## O que conta como tempo de jogo

O temporizador é do console: o PlayGuard o lê, ele mesmo não conta nada. O que se sabe, e com que certeza ([parental-controls.md](../parental-controls.md#time-spent-and-time-left), em inglês, tem as medições):

| Situação | Conta? | Com que certeza |
|---|---|---|
| Um jogo rodando | Sim | Medido (22.0.0) |
| **Um jogo deixado aberto no menu HOME** (suspenso em segundo plano) | **Sim**: feche-o, ou coloque o console no modo de espera | Relatado por pais ([Arqade](https://gaming.stackexchange.com/questions/398954/)); condiz com as medições (o console conta enquanto um aplicativo está aberto) |
| Modo de espera | Não | Relatado ([Arqade](https://gaming.stackexchange.com/questions/398954/)); não medido pelo PlayGuard |
| **PlayGuard aberto por cima de um jogo** (como aplicativo) | **Sim**, como esse jogo | Medido (22.0.0): abra-o com o usuário de um responsável, ou pelo álbum para uma olhada rápida |
| PlayGuard aberto pelo álbum (modo applet) | Não | Deduzido: ele não é um aplicativo, e o tempo fora dos aplicativos não pareceu contar |
| Enquanto o controle parental está desbloqueado temporariamente | Talvez não | Não medido: ainda em aberto |
| Depois que o relógio foi alterado | O dia recomeça; depois de um salto para trás, a contagem para até o relógio passar do horário antigo | Observado (22.0.0); *Hoje* avisa quando o console não está contando |
