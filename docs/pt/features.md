<!-- translated from docs/features.md, sha256 06fe889ee4f2 -->
# Funcionalidades

O que faz cada separador, como o PlayGuard escreve em segurança e o que a consola conta como tempo de jogo. Novo no PlayGuard? Comece por [como começar](getting-started.md). Voltar ao [README](../../README.md).

<!-- github-only: the website shows each language's screenshots, in the page's theme -->

As capturas de ecrã mostram o tema escuro, o predefinido do PlayGuard (em inglês).

| Hoje | Consola |
|---|---|
| ![Hoje](../../images/screenshots/dashboard.png) | ![Consola](../../images/screenshots/console.png) |
| **Tempo de jogo** | **Tempo de jogo › um dia em edição** |
| ![Tempo de jogo](../../images/screenshots/play_timer.png) | ![Um dia em edição](../../images/screenshots/per_day.png) |
| **Atividade** | **Um jogo** |
| ![Atividade](../../images/screenshots/activity.png) | ![Um jogo](../../images/screenshots/activity_game.png) |
| **Restrições** | **Consola › Relógio de rede** |
| ![Restrições](../../images/screenshots/restrictions.png) | ![Relógio de rede](../../images/screenshots/clock.png) |
| **Segurança** | **Preferências** |
| ![Segurança](../../images/screenshots/security.png) | ![Preferências](../../images/screenshots/preferences.png) |

## Comandos

| Botão | O que faz |
|---|---|
| ↑/↓ | Mover |
| Ⓐ | Confirmar |
| Ⓑ | Voltar ou cancelar (duas vezes na barra lateral para sair) |
| Ⓧ | Atualizar (nos separadores que leem a consola) |
| Ⓨ | Notificações, a partir de qualquer separador |
| **−** | Idioma e tema, a partir de qualquer separador (no rodapé, omitidos primeiro quando os botões do próprio ecrã precisam do espaço) |
| **+** | Guarda os limites por dia (e a semana editada em *Tempo de jogo*) |
| **ZR** | *Anular*, durante alguns segundos depois de guardada uma alteração |

A aplicação está organizada em separadores, como as Definições da consola; cada um tem a sua secção abaixo.

- **O que falhou é dito numa caixa de diálogo, o que correu bem numa mensagem breve.** Uma caixa de diálogo só diz o que se aplica a esta consola neste momento: que o jogo em curso seria suspenso, só enquanto há um jogo aberto; que a aplicação móvel poderia substituir uma alteração, só enquanto está associada e consegue sincronizar.
- **O cabeçalho** indica, em todos os separadores, quando o controlo parental está desbloqueado temporariamente (a sua etiqueta), e quando a aplicação está só de leitura (a etiqueta diz então *Só de leitura*, e as ações que escreveriam ficam a cinzento e são recusadas, indicando onde se desativa); tocar-lhe leva ao sítio onde isso se desfaz (*Hoje › Reativar*, o ecrã do firmware ou *Ajuda e diagnóstico*). O rodapé indica os botões que os abrem a partir de qualquer separador: ⊖ para o idioma e o tema, Ⓨ para as notificações (com o seu número).
- **No modo só de leitura**, as ações ficam no lugar, a cinzento, e dizem porquê quando são premidas.

## O aspeto: Cadran

O aspeto do PlayGuard chama-se Cadran: uma barra de ícones para os separadores, uma paleta azul-marinho ou branco frio (consoante o tema), cartões em vez de listas. Cada separador abre no que mais importa (um anel, mosaicos, cartões, botões), com as outras definições por baixo.

- **A etiqueta do cabeçalho** diz de que trata o separador (*Controlo parental ativo*, *Temporizador ativo*…); enquanto o controlo parental está desbloqueado, di-lo em todos os separadores, e tocar-lhe abre *Hoje › Reativar*; enquanto está em pausa para si (veja [abaixo](#o-pin-põe-o-controlo-parental-em-pausa-enquanto-está-no-playguard)), diz *Controlo parental em pausa*, com calma, e tocar-lhe explica o que isso é. O texto secundário nunca fica abaixo de 15 px, para se ler do sofá numa TV. O relógio do rodapé mostra as horas e os minutos (*16:00*).
- **As caixas de diálogo** são cartões com os botões em forma de pílula, a confirmação a verde-azulado (a coral quando remove ou interrompe alguma coisa).
  - As escolhas abrem-se num cartão por cima das indicações dos botões, a atual sobre um verde-azulado suave com a sua marca de verificação. O contorno verde-azulado é só do foco, em todo o lado: um nível, uma conta, uma escolha em vigor mantém o fundo suave.
  - Uma caixa de diálogo longa esbate-se em baixo enquanto há mais a seguir, e o seu aviso (um jogo prestes a ser suspenso) vem primeiro, num cartão âmbar.
  - Uma falha tem como título o que falhou, o código de erro em pequeno por baixo do motivo, e propõe *Enviar um relatório* quando ajuda.
  - As mensagens aparecem como cartões por baixo do cabeçalho, com uma marca de verificação verde-azulada para o que foi feito e uma marca âmbar para o que foi recusado.
- **Os outros ecrãs** ganham o mesmo aspeto: o editor por dia de um perfil mostra a semana como em *Tempo de jogo* (editada da mesma forma, com os preenchimentos rápidos em mosaicos); *Jogos instalados* e *Todos os jogos* põem as suas linhas em cartões; *Perfis* e *Ajuda e diagnóstico* dão a cada linha o seu ícone.
- **Tamanho do texto** (*Preferências*, ou **AA** no cabeçalho):
  - *Automático*, a predefinição, aumenta todos os textos em 25 % enquanto a consola está na base, na TV, e repõe-nos no modo portátil; *Maior* mantém-nos maiores, *Normal* nunca.
  - Os nomes dos separadores da barra lateral também crescem, igualmente 25 %; o título, a etiqueta e a data do cabeçalho e as indicações dos botões e o relógio do rodapé crescem um pouco menos (15 a 20 %), pois as suas linhas já estão cheias. Um nome demasiado longo para o seu espaço numa língua é desenhado um pouco mais pequeno, nunca menos do que no tamanho normal, em vez de ser cortado. Os nomes dos níveis de Restrições mantêm o seu tamanho, que já é grande.
  - A mudança é imediata, em todos os ecrãs, sem reiniciar.
  - Os 25 % vêm de uma TV de 55" em 4K: o texto mais pequeno (15 px) tem então 14,6 mm de altura em maiúsculas, 20′ de arco a 2,5 m, onde a leitura é confortável (16′ no tamanho normal, o mínimo indispensável).
- **Efeitos visuais** (*Preferências*): *Automático: completos na TV*, a predefinição, mantém o halo de fundo e todos os movimentos do visual enquanto a consola está na base: os ecrãs e as suas linhas a subir para o lugar, o anel a desenhar-se e a deslizar até um novo valor, a pulsação do anel do ecrã da criança nos últimos minutos, as pequenas respostas a um toque num botão. Na mão (modo portátil ou de mesa, a carregar ou não) deixa-os todos de fora, e voltar a pôr a consola na base trá-los de volta. *Sempre completos* mantém-nos também na mão. *Menos movimento* deixa de fora todos os movimentos decorativos, também na base, e mantém o halo: para quem se cansa com o movimento.
- **Anular:** depois de guardada uma alteração, **ZR** *Anular* fica no rodapé durante alguns segundos: abre essa alteração no histórico de alterações, para repor o valor anterior. Um histórico de alterações vazio di-lo num cartão.

## Notificações — o que precisa de si, a partir de qualquer separador

Um sino no cabeçalho, à esquerda da versão, reúne o que precisa da intervenção do pai ou da mãe; Ⓨ abre a lista a partir de qualquer separador (o contador do sino indica *2* quando há duas coisas a fazer).

À esquerda do sino, o idioma (o seu código, *FR*, *EN*…) e o tema (um sol para claro, uma lua para escuro, um círculo meio preenchido quando segue a consola), depois o tamanho do texto (**AA**, com o A grande a verde-azulado enquanto o texto está maior): um toque abre a mesma lista que em *Preferências*, e **−** propõe-nos a partir de qualquer separador. Um novo tema ou idioma tem efeito de imediato, com cada ecrã refeito no mesmo separador e o foco na mesma linha, chinês e o seu próprio tipo de letra incluídos.

### A fazer

Cada um com um sinal de aviso âmbar, contado a âmbar no sino; fica até ser resolvido:

- ainda sem PIN, ou um PIN que qualquer pessoa experimentaria primeiro (0000, 1234, 1212, 2580…: avaliado na consola, nunca mostrado);
- um PIN na consola enquanto o próprio PlayGuard abre sem ele (*Proteger o PlayGuard com o PIN* em *Nunca*);
- o controlo parental desbloqueado temporariamente (*Reativar*);
- tempo extra de um dia anterior ainda por retirar;
- o alarme "Tempo esgotado" desativado;
- alterações feitas fora do PlayGuard;
- a consola a não contar o tempo de jogo;
- um relógio de rede impreciso.

### Para informação

Cada um com um "i", um ponto no sino; pode ser marcado como lido:

- um limite ainda reduzido por *Terminar o jogo por hoje* num dia anterior (*Repor*);
- uma nova versão instalada (*Novidades*; depois de uma atualização no lugar, a compilação anterior mantida), ou uma versão mais recente do PlayGuard;
- um relatório enviado online (*Mostrar a ligação*), ou o PlayGuard fechado inesperadamente da última vez (*Enviar um relatório*);
- o tempo de jogo da semana (a cada 7 dias até ser lido; com a comparação, por dia, com a semana anterior), e um limite ultrapassado em dois ou mais dos últimos 7 dias quando a consola só avisa;
- uma cópia de segurança das definições por fazer (nenhuma ainda, ou nenhuma há 30 dias);
- todos os jogos bloqueados há uma semana;
- uma nova conta de utilizador;
- a aplicação móvel associada;
- o relógio de rede encontrado desacertado ao arrancar.

### Consola

As entradas sobre a consola, e não sobre o controlo parental, vêm no fim, numa linha que Ⓐ abre (aberta logo quando não há mais nada). As que são a fazer são contadas no distintivo do separador Consola, não no sino:

- jogos instalados que não arrancam (*Verificar os jogos instalados*), patches de jogos a rever;
- o cartão SD quase cheio (menos de 500 MB);
- um dos ficheiros do próprio PlayGuard que não pôde ser posto de lado ou escrito no cartão SD;
- o sysmodule de recuperação desatualizado (não é o que vem com esta versão, ou foi instalado antes de indicar a sua versão; *Atualizar* instala o que esta versão traz), depois a consola a reiniciar para que ele funcione, até ser reiniciada;
- uma compilação que não acabou de arrancar e da qual se voltou atrás (*Enviar um relatório*).

As de informação: uma compilação da qual se voltou atrás, um firmware não testado com esta versão, uma versão mais recente do Atmosphère, do hekate ou do firmware (verificada com *Procurar atualizações todos os dias*; a entrada do firmware diz para esperar até o Atmosphère o suportar), uma versão mais recente da integração do Home Assistant (depois de esta ter indicado que versão o Home Assistant tem: *Novidades* mostra o que mudou desde então e o código QR que a abre no HACS), o número de série não ocultado no emuMMC, o controlo parental próprio do outro sistema, um dos ficheiros do próprio PlayGuard encontrado danificado e posto de lado (uma mensagem breve ao arrancar remete para ela).

### Como se comportam as entradas

- Cada secção lista as suas entradas por uma ordem fixa, seja qual for a verificação que terminou primeiro.
- **Na barra lateral**, um distintivo âmbar junto do nome de um separador conta as suas entradas a fazer, onde se resolvem: o separador Segurança (sem PIN, um PIN fraco, o PlayGuard sem o seu PIN, desbloqueado temporariamente), o Tempo de jogo (o alarme desativado, a consola a não contar), *Hoje* (alterações feitas fora do PlayGuard, tempo extra por retirar), a Consola (as suas entradas acima, e o relógio de rede). As entradas ignoradas e as de informação não são contadas.
- No separador, o mesmo distintivo está na linha onde cada uma se resolve (*Relógio de rede*, *Jogos instalados*, *Armazenamento*, *Patches de jogos*; *Definir ou alterar o PIN*, *Proteger o PlayGuard com o PIN*; *Avançado* do Tempo de jogo para o alarme), e o separador abre na primeira delas. Uma linha sem valor próprio diz antes, a âmbar, para que serve o distintivo (*Fácil de adivinhar*, *Não é preciso*).
- **Ⓐ** numa entrada faz a correção (*Ligá-lo*, *Acertá-lo*, *Atualizar*…) ou abre o separador onde ela se faz. Uma entrada desaparece sozinha assim que o que ela indica fica resolvido.
- **Ⓧ** marca uma entrada de informação como lida; **Ⓨ** marca-as todas. Uma entrada marcada como lida continua lida no arranque seguinte, até o seu texto mudar.
- **Ignorar:** algumas entradas a fazer podem ser ignoradas com Ⓧ, depois de uma confirmação e do PIN: um PIN que qualquer pessoa experimentaria primeiro, o PlayGuard a abrir sem o PIN, o alarme "Tempo esgotado" desativado, o cartão SD quase cheio (nunca a falta de PIN, o controlo desbloqueado, o relógio, o temporizador a não contar nem os ficheiros do próprio PlayGuard). Uma entrada ignorada deixa de ser contada e aparece no fim em *Ignorados*, onde Ⓐ a volta a contar. Regressa sozinha 30 dias depois (a menos que *Voltar a contar os avisos ignorados após 30 dias* esteja desativado), quando o que ela indica fica resolvido e depois volta a acontecer, ou, para o PIN, com um novo PIN de outro comprimento.
- *Preferências › Notificações* desativa três tipos: versões do Atmosphère, do hekate e do firmware; o tempo de jogo da semana; avisos (cópias, segundo sistema da consola, jogos bloqueados).
- O estado por trás disto é lido quando o PlayGuard arranca, a cada minuto no ecrã principal e a cada 5 s enquanto *Hoje* é mostrado.

## Hoje — o dia num relance

*[Experimentar no simulador ›](https://guard.boreale.dev/simulator/)*

### O anel e a linha

**O tempo restante** num anel: um dia sem jogo diz *Sem jogar*, não *0 min*; o tempo extra dado hoje aparece como um segmento mais claro no fim, e o anel desloca-se até cada novo valor.

Por baixo, o que interrompe o jogo primeiro:

- antes do fim do tempo ou do alarme de hora de dormir, o resto do dia como uma linha, com o momento atual à esquerda, cada um deles uma marca com a sua hora (*16:40 tempo esgotado*, *21:00 hora de dormir*), a verde-azulado até à primeira, a âmbar nos seus últimos 15 minutos;
- caso contrário, por palavras: o alarme de hora de dormir depois de esgotado o tempo do dia, a hora a partir da qual se pode voltar a jogar; enquanto o controlo parental está desbloqueado, o limite de hoje em pausa e porquê.

### As ações

- **Tempo extra**, as suas quantidades numa fila (+15 min, +30 min, +1 h), cada uma à distância de um toque, depois **…** para qualquer outra (uma duração, outro dia): uma quantidade selecionada desloca na linha o fim do tempo (*com mais 15 min*), ou diz na marca do alarme de hora de dormir que este continua a vir primeiro; a primeira destaca-se a verde-azulado quando restam 15 min ou menos, ou nada.
- **Terminar por hoje**, **Mostrar à criança**.
- **Limite de hoje…**: o limite de hoje escolhido ali mesmo, escrito depois de uma confirmação.
- **O perfil em vigor**, pelo seu nome, depois de guardado um perfil (*Perfis…* quando nenhum corresponde): Ⓐ lista-os, com o que está em vigor assinalado, para aplicar outro depois de uma confirmação (os dias que mudam), ou *Gerir os perfis…*.
- Enquanto o controlo parental está desbloqueado, **Reativar** vem primeiro, a toda a largura da fila, e *Tempo extra* sai do caminho.
- Sem limite em nenhum dia, **Definir os limites da semana** abre a semana em *Tempo de jogo*.

### Os cartões e as etiquetas

- O **alarme de hora de dormir** (a hora a partir da qual se pode voltar a jogar, ou daqui a quanto tempo toca quando a consola não o diz; desativado, *Desativado* em grande e Ⓐ *Defini-lo*), o **controlo parental** e o **relógio de rede**, em cartões.
- O estado da consola em **etiquetas**:
  - uma diz se o redirecionamento DNS do Atmosphère bloqueia os servidores da Nintendo, e Ⓐ nela diz quais e o que isso muda: sem sincronização para a aplicação móvel, um relógio de rede que já não se acerta sozinho;
  - outra, quando aberto como aplicação, que o tempo passado no PlayGuard conta: como esse jogo quando aberto sobre um jogo (*Aberto a partir de um jogo*), com o nome do seu ícone quando iniciado a partir de um forwarder no menu HOME (*Aberto pelo seu ícone HOME*), com Ⓐ a explicar como;
  - um jogo deixado aberto no menu HOME aparece como uma etiqueta âmbar (*continua a contar*, com Ⓐ a explicar porquê), quando a etiqueta sobre o tempo do próprio PlayGuard ainda não o diz;
  - nenhuma das duas enquanto o controlo parental está em pausa para si: o tempo passado aqui não é então o da criança.
- As etiquetas põem primeiro o que precisa do pai ou da mãe (âmbar, com um triângulo), depois o que está ativo (verde-azulado, com uma marca de verificação), depois o resto (cinzento, com um ponto): a forma diz o mesmo que a cor.
- O que está bem e o que é só informado juntam-se numa única etiqueta depois das que precisam do pai ou da mãe (*✓ 3 verificações em ordem*); Ⓐ sobre ela mostra-os, enquanto o separador estiver aberto.
- Por baixo das etiquetas, o tempo extra ainda por retirar (Ⓐ retira-o) e o alarme "Tempo esgotado" enquanto está desativado (Ⓐ volta a ativá-lo).

### Primeiros passos

Até o PIN e os limites da semana estarem definidos, um cartão por cima dos cartões lista os três passos (o PIN, um limite diário, o relógio de rede), cada um assinalado a verde-azulado quando feito ou rodeado a âmbar, com quantos estão feitos (*1 de 3*); Ⓐ num deles fá-lo ali mesmo.

### A consola está a contar?

O tempo do próprio PlayGuard conta enquanto ele corre como aplicação, por isso, com um limite hoje, o tempo restante deve descer enquanto está aberto. Quando não se mexe durante cerca de 90 s no ecrã (temporizador ativo, um limite hoje, sem desbloqueio nem pausa para si, não aberto a partir do álbum), uma linha âmbar por baixo do tempo jogado indica *A consola não está a contar o tempo de jogo neste momento*, com as causas prováveis:

- o relógio foi atrasado (volta a contar mais tarde);
- o relógio de rede nunca foi acertado (*Consola › Relógio de rede*);
- acabou de ser escrito um limite (inicie um jogo durante um minuto e verifique de novo).

Desaparece assim que o tempo restante volta a mexer. O Tempo de jogo também o indica.

### Alterado fora do PlayGuard

O tempo de jogo de hoje a recomeçar do zero no mesmo dia (uma mudança de relógio faz isso), o relógio da consola mudado em mais de 5 minutos, limites diferentes dos que o PlayGuard viu da última vez (a aplicação móvel, outra ferramenta): um cartão âmbar por cima do anel até ser dispensado (Ⓐ) ou até ao dia seguinte, e uma entrada no histórico de alterações. O PlayGuard não corre em segundo plano: só vê o que mudou enquanto estava aberto, ou desde a última vez que esteve, quando volta a ler a consola.

### No limite

A consola mostra "Tempo esgotado" e depois suspende o jogo (sem "continuar") ou apenas avisa: *Quando o tempo acabar* mostra qual e permite mudá-lo, depois de definido um limite ou uma hora de dormir ([detalhes, em inglês](../parental-controls.md#when-the-time-runs-out)).

### Tempo extra e terminar o jogo por hoje

- **Tempo extra hoje**: +15 min, +30 min, +1 h por predefinição.
- **Terminar o jogo por hoje**: limite 0 só para hoje, *Agora* ou *Daqui a 5 minutos* quando o tempo jogado é conhecido (o limite passa então a ser o tempo jogado + 5 min, o tempo de guardar). No bloqueio aparece o "Tempo esgotado" da consola, e o progresso não guardado pode perder-se — a confirmação avisa.
- **No dia seguinte** — ao arrancar, ou à meia-noite se a aplicação estiver aberta — o PlayGuard repõe o limite habitual:
  - depois de tempo extra, sozinho, sem perguntar e sem PIN (só baixa o limite, como reativar o controlo parental), desbloqueando por um instante se o temporizador estiver a contar e voltando logo a bloquear. Espera enquanto não puder escrever (modo só de leitura, um firmware que não suporta, *Bloquear todos os jogos* ativo) e, uma semana depois no mesmo dia, enquanto o limite mais baixo pararia o jogo em curso (então no dia seguinte). Para manter mais tempo num dia, altere o limite desse dia;
  - depois de *Terminar o jogo por hoje*, pergunta se deve manter 0 ou repor o limite (com o PIN, como qualquer alteração que sobe um limite), ou repõe-no como qualquer alteração (*Tempo de jogo › Repor o limite após "Terminar o jogo por hoje"*); repor "sem limite" é sempre perguntado.
- **Tempo extra noutro dia** (*Tempo extra › Outro dia…*, no fim da sua lista): amanhã ou um dos cinco dias seguintes, cada um listado com o seu limite, e depois a quantidade. O limite desse dia é aumentado já, só para essa data, e reposto sozinho (só baixa o limite) da próxima vez que o PlayGuard for aberto depois de o dia terminar — não numa consola onde o PlayGuard não é aberto, onde o mesmo dia da semana seguinte também o mantém. Um de cada vez: a lista propõe então retirá-lo (com o limite reposto de imediato). Uma alteração do limite desse dia entretanto (a semana, um perfil…) prevalece: nada é reposto.

### O ecrã da criança

*Mostrar à criança* em *Hoje*, para entregar a consola:

- só o tempo restante (enquanto o controlo parental está desbloqueado, o limite de hoje, em pausa), a hora a que o jogo termina (*Termina por volta das 16:40*; quando o alarme de hora de dormir vem primeiro, o anel faz antes a contagem decrescente até ele, *45 min antes de dormir*, como na última hora de um dia sem limite), a hora de dormir e o que acontece quando o tempo acaba;
- feito para ser lido do outro lado da sala, na TV; o seu texto mantém o tamanho seja qual for o tamanho do texto (só o título e o rodapé o seguem, como em todos os ecrãs);
- nos últimos 5 minutos (de jogo ou antes da hora de dormir), o fim passa a âmbar e o anel pulsa devagar; a hora de dormir passa a âmbar nos seus últimos 30;
- Ⓐ e Ⓨ, os botões do adulto, aparecem em pequeno no canto superior direito, para que o rodapé mostre só o da criança;
- Ⓐ volta aos ecrãs do adulto com o PIN; Ⓨ dá tempo extra a partir dali, depois do PIN (as quantidades de *Tempo extra*, sem sair do ecrã da criança); Ⓑ sai do PlayGuard (perguntando primeiro se deve reativar o controlo parental, como ao sair).

Com o texto maior, *Hoje* continua a caber num só ecrã: a linha por cima do título só indica o dia e o seu limite (*QUINTA-FEIRA · 2 h*), e os botões e os cartões ficam um pouco mais apertados. Atualiza a cada 5 s (Ⓧ atualiza já).

## Tempo de jogo — a semana é o editor

*[Experimentar no simulador ›](https://guard.boreale.dev/simulator/?tab=play_timer)*

### Editar a semana

Ⓐ num dia edita-o:

| Botão | Na edição |
|---|---|
| ↑/↓ | Alterá-lo em 15 min, de hora em hora quando mantidos premidos |
| ←/→ | Passar ao dia seguinte |
| Ⓛ | Copiá-lo para segunda–sexta |
| Ⓡ | Copiá-lo para o fim de semana |
| Ⓨ | Sem limite |
| Ⓧ | Uma duração exata (*Introduzir uma duração…*: ↑/↓ de 15 em 15 min, de hora em hora quando mantidos premidos, Ⓛ/Ⓡ de 5 em 5 min, com OK selecionado) |
| Ⓐ ou Ⓑ | Terminar a edição (Ⓐ diz que dia: *Concluir segunda-feira*; ainda nada fica guardado) |

- Por toque, um dia tocado fica em edição, e depois um toque na sua parte de cima ou de baixo altera-o em 15 min.
- As alterações são guardadas em conjunto, com **+** ou *Guardar*, como no editor por dia (a confirmação lista os dias alterados, antes → depois); *Descartar* anula-as.
- Dentro da barra de cada dia, o tempo jogado na sua última ocorrência (incluindo hoje, todas as contas). Acima do limite, a barra fica cheia e o topo a coral, na mesma proporção que o tempo acima do limite tem no tempo jogado; uma legenda por baixo da semana di-lo (*limite ultrapassado*), e o limite também fica a coral. Os dias depois de hoje mostram o jogo da semana passada, mais esbatidos (selecionado: *Sexta-feira anterior: 2 h 10 jogado de 2 h*).
- Um dia alterado e ainda não guardado tem o limite a âmbar e a barra contornada a âmbar; por baixo da semana, os números do dia selecionado (*Quinta-feira: 1 h 20 jogado de 2 h*).
- Por cima da semana, a âmbar, um limite atingido e o que a consola faz nesse caso, ou a consola a não contar o tempo de jogo.
- `0` minutos significa *sem jogo nesse dia*; *Remover o limite de tempo de jogo* desliga o temporizador.

### Botões rápidos e definições rápidas

**Botões rápidos** por cima e por baixo da semana abrem logo as suas escolhas: os perfis (quantos estão guardados), o mesmo limite todos os dias, o alarme de hora de dormir e *Quando o tempo acabar*. *Remover o limite* é a última linha do separador, à parte (o histórico desfá-lo).

- **Mesmo limite todos os dias:** uma lista rápida ou uma duração exata (*Introduzir uma duração…*: ↑/↓ de 15 em 15 min, de hora em hora quando mantidos premidos, Ⓛ/Ⓡ de 5 em 5 min).
- **Dias úteis e fim de semana ›:** o editor, que pede o limite de segunda a sexta e depois o do fim de semana.
- **Tempo extra ou terminar o jogo hoje ›** abre *Hoje*, onde estão ambos. Uma nota por baixo diz o que a consola faz no limite. Enquanto o controlo parental está desbloqueado temporariamente, o tempo de jogo *pode* não ser contado: a questão continua em aberto ([documentação, em inglês](../parental-controls.md#still-open)).
- Cada confirmação lista os dias que mudam, antes → depois.

### Perfis

- **Perfis** guardados no cartão SD: aplicar (um perfil já em vigor limita-se a dizê-lo), editar, mudar o nome ou eliminar um; guardar os limites atuais ou criar um novo.
- Qualquer nome, em qualquer escrita (*École*, *周末*, *Выходные* …) — um nome sem letras latinas recebe um nome de ficheiro próprio, e dois nomes que dariam o mesmo ficheiro são detetados.
- **Perfil agendado** (*Perfis › Perfil agendado*): *Com o Home Assistant*. O PlayGuard só funciona enquanto está aberto, por isso não consegue mudar de perfil sozinho no dia certo: o Home Assistant fá-lo, a partir de um calendário ou das férias escolares, depois de *Consola › Acesso remoto* lhe ligar a consola (uma automação que define o seu seletor *Perfil*; [o guia do Home Assistant](../home-assistant.md), em inglês). Um perfil agendado com uma versão anterior continua a ser proposto quando o seu período começa, e a linha continua a alterá-lo, removê-lo ou terminá-lo.

### Bloquear todos os jogos

*Bloquear todos os jogos*, por baixo das definições da semana: um interruptor que põe o limite de cada dia a 0, para que seja preciso o PIN para iniciar um jogo — um bloqueio leve, sem classificações etárias nem limites de comunicação.

- Bloqueia o arranque dos jogos, não o menu HOME, e precisa de um PIN.
- Os limites anteriores voltam quando é desativado.
- Enquanto está ativo, *tempo extra* e *terminar o jogo por hoje* são recusados, e os limites definidos de outra forma (um perfil, uma cópia de segurança, o histórico…) substituem-no.

### Alarme de hora de dormir

- A hora do alarme (16:00 a 23:45, ou *Desativado*) e a hora a partir da qual se pode voltar a jogar (05:00 a 09:00), igual todos os dias, definidas com o manípulo: para cima e para baixo um quarto de hora, L e R uma hora.
- Ou, com Ⓨ *Por dia* na caixa de diálogo do alarme, de segunda a sexta, o fim de semana ou um só dia, mantendo os outros dias a sua (cada conjunto de dias listado com a sua hora). Quando os dias diferem, o separador indica *Varia consoante o dia*, e a semana em *Tempo de jogo* mostra o alarme de cada dia por baixo do seu nome.
- O seu lugar nas definições do temporizador, deduzido das definições da aplicação móvel, está verificado numa consola (22.0.0); mesmo assim, o PlayGuard só o altera quando a consola indica o que o PlayGuard lê ali, verifica a resposta da consola depois da alteração e repõe as definições anteriores se for diferente.

### Definições do tempo extra e Avançado

- **Tempo extra:** as quantidades propostas (+15/+30/+1 h, +10/+20/+30 min …) e repor o limite habitual sem perguntar depois de *Terminar o jogo por hoje* (o tempo extra é sempre retirado sem perguntar). Ambos são feitos da próxima vez que o PlayGuard é aberto, a partir do dia seguinte: uma consola onde não é aberto mantém o limite desse dia.
- **Avançado ›**, um ecrã próprio: o interruptor das ações de classe de depuração e, enquanto está ativo, o alarme "Tempo esgotado" ligado/desligado e pausar / retomar a contagem decrescente. O mesmo interruptor permite que uma cópia de segurança ou o histórico reponham o alarme.

## Atividade — quem jogou a quê, e durante quanto tempo

*[Experimentar no simulador ›](https://guard.boreale.dev/simulator/?tab=activity)*

Tempo por jogo **hoje**, nos **últimos 7 dias** e **no total**, a partir do próprio registo de atividade da consola — para todas as contas ou **uma conta de utilizador** (as contas em botões no topo, quando a consola tem várias). O PlayGuard abre na conta mostrada da última vez, ou na definida em *Conta mostrada primeiro* (todas as contas, ou uma; no fundo do separador, com *Todos os jogos*).

### No topo

- As contas em botões, o período e *Exportar*.
- Hoje, os últimos 7 dias e desde sempre em números. Por baixo dos últimos 7 dias, a comparação, por dia, com os 7 anteriores: ▲ *+20 min/dia vs semana anterior* a âmbar, ▼ menos a verde-azulado, *como na semana anterior* com uma diferença de até 5 minutos.
- Os quatro jogos mais jogados em cartões (Ⓐ abre um); nos últimos 7 dias, aquele cujo tempo mais aumentou em relação à semana anterior diz em quanto, a partir de meia hora (▲ *+2 h vs semana anterior*).
- Nada jogado no período: um cartão di-lo, com *Ver desde sempre* quando há alguma coisa para ver.

### Gráficos e resumo

- **Um gráfico dos últimos sete dias** (hoje à direita, uma legenda por baixo), com o limite desta semana para cada dia como uma linha e o tempo acima dele a âmbar, marcado com !; totais de hoje, da semana e de sempre.
- **Um resumo** do período escolhido (hoje, os últimos 7 dias, os **últimos 30 dias** ou desde sempre), para a conta mostrada: média por dia, o jogo mais jogado, os dias com jogo e o mais cheio (últimos 7 dias), a sessão média (desde sempre).
- **Horas de jogo, últimos 7 dias**: um quadrado para cada hora de cada dia, segunda-feira primeiro, mais intenso quanto mais se jogou (todos os jogos, a conta mostrada); depois as sessões (jogar com pausas de menos de 5 minutos conta como uma só sessão, mesmo mudando de jogo) e a sua duração média, a mais longa e o seu dia, as duas horas seguidas com mais jogo, e o tempo jogado entre as 21:00 e as 06:00.

### Todos os jogos e a página de um jogo

- **Todos os jogos**, num ecrã próprio: cada jogo jogado no período, os mais jogados primeiro, como **grelha de capas** ou **lista** (Ⓨ alterna, e a escolha é mantida), com os números do jogo selecionado sempre no topo: o seu ícone e nome, o tempo de jogo no período, desde sempre, os arranques e a última vez que foi jogado. Ⓧ escolhe o período (o separador Atividade acompanha).
- Os ícones são lidos à volta do jogo selecionado (não no modo applet, para poupar memória); um comando cinzento ocupa o lugar até um chegar, e para um jogo sem ícone.
- Uma biblioteca grande mostra os primeiros 50 jogos e depois *Mostrar todos os jogos* (100 jogos no máximo no modo applet); a exportação contém-nos sempre todos. O PlayGuard lê 256 jogos no máximo (os desta semana primeiro): para além disso, o separador Atividade, Todos os jogos e a exportação (exceto CSV) dizem que faltam alguns, e o total de sempre não os inclui.
- **Ⓐ num jogo:** os seus últimos sete dias em barras, arranques, primeira e última vez que foi jogado, tempo por conta de utilizador. Os jogos eliminados mantêm os seus números de sempre, mesmo um eliminado antes de o PlayGuard ser instalado, desde que o registo de jogo da consola ainda o nomeie, e o seu nome desde que o PlayGuard o tenha visto (senão *Jogo eliminado* e o seu ID do título).

### Contas

- O tempo de um jogo que nenhuma conta de utilizador tem (alguns jogos nunca pedem uma, ou a conta foi eliminada entretanto) aparece como *Sem conta*, a partir de um minuto.
- Uma conta de utilizador que o PlayGuard nunca tinha visto é assinalada a âmbar no separador durante 30 dias (*Conta nova desde …*: uma conta nova é uma forma conhecida de recomeçar do zero) e registada no histórico de alterações; as contas que existem quando o PlayGuard as lista pela primeira vez não são assinaladas.

### Exportação

- **Exportação para o cartão SD** (*Exportar*) em CSV, JSON, XLSX (Excel) ou PDF, uma coluna por dia, e depois os últimos 7 dias, os últimos 30 dias e desde sempre.
- Abre logo com os últimos números lidos (guardados no cartão SD entre utilizações), atualizados em segundo plano quando têm mais de um minuto; Ⓧ volta a lê-los já, com um indicador de carregamento.
- Os tempos são aproximados se o relógio da consola foi alterado.

## Restrições — nível, classificação etária, comunicação

*[Experimentar no simulador ›](https://guard.boreale.dev/simulator/?tab=restrictions)*

- **O nível de restrição:** Criança pequena, Criança, Adolescente e Personalizado lado a lado em mosaicos, cada um com o que restringe (um cadeado coral para o que é restrito, um visto verde no resto, e os jogos instalados que bloqueia; Ⓐ escolhe-o; com *Personalizado* em vigor, Ⓐ leva às suas definições, mais abaixo); *Nenhum* por baixo deles. O nível em vigor diz *✓ Atual*; o contorno verde-azulado só está onde está o foco. O que um nível restringe (limite de idade, publicações, comunicação) é dito de novo antes de ser aplicado; *Nenhum* pergunta a coral, pois levanta todas as restrições.
- **As definições do nível:** classificação etária, publicação nas redes sociais, comunicação com outras pessoas. Num nível predefinido, são as dele: alterar uma torna o nível *Personalizado*, a partir das definições do nível predefinido com essa alteração, depois de perguntar (com as definições listadas).
- **Em *Classificação etária*,** o **organismo de classificação** (PEGI, ESRB, USK, CERO …): a lista diz para que países cada um classifica (PEGI: a maior parte da Europa, ESRB: Estados Unidos, Canadá e América Latina, USK: Alemanha, PEGI BBFC: Reino Unido…).
- **Jogos instalados bloqueados** (também em *Classificação etária*): quantos dos jogos instalados o nível em vigor bloqueia pela sua classificação etária (*3 de 15*), com Ⓐ a listá-los com a sua classificação (*PEGI 16*); o mosaico de cada nível também o indica (*Jogos bloqueados · 3*).
  - As classificações são as de cada jogo (os seus dados de controlo, lidos pela verificação dos jogos instalados ao arrancar ou a partir de *Consola › Jogos instalados*), segundo o organismo que a consola usa.
  - Um jogo que esse organismo não classifica não é contado, nem as exceções definidas na aplicação móvel.
  - O ecrã de um jogo em Atividade mostra a sua classificação, e *bloqueado no nível atual* quando é o caso.
- **Em *Outros*,** o modo VR. Os jogos autorizados a comunicar estão no relatório de diagnóstico, em bruto, até a sua lista ser compreendida.

## Segurança — o PIN, o desbloqueio, o menu homebrew, a aplicação móvel

*[Experimentar no simulador ›](https://guard.boreale.dev/simulator/?tab=security)*

- **No topo**, quatro mosaicos: o PIN, as restrições, o desbloqueio temporário e o menu homebrew, cada um com uma marca de verificação verde-azulada quando está como deve estar (um PIN definido, restrições ativas, bloqueado) ou um sinal de aviso âmbar quando é preciso intervir; o menu homebrew indica *Aberto* ou *Bloqueado* (uma marca de verificação depois de bloqueado: aberto não é problema sem forwarders).
- **Definir ou alterar o PIN** (ecrã de PIN do sistema).
- **Mostrar o PIN**: depois de um aviso e do próprio PIN, pedido sempre; registado no histórico, e depois é proposto um novo PIN.
- **Desbloquear temporariamente**, e se o PlayGuard o reativa sozinho logo depois de uma alteração que precisou do desbloqueio (ativo por predefinição).

### Proteger o PlayGuard com o PIN

O PIN no próprio PlayGuard:

| Definição | Efeito |
|---|---|
| *Nunca* | Nenhum PIN no PlayGuard (o sino assinala-o enquanto a consola tiver um) |
| *Antes de uma alteração* (predefinição) | Qualquer pessoa pode ver, só o pai ou a mãe altera alguma coisa |
| *Para abrir o PlayGuard* | Um ecrã de bloqueio ao arrancar |

- Depois de introduzido, o PIN vale até o PlayGuard fechar; *Pedir o PIN novamente após 5 minutos* (desativado por predefinição) volta a pedi-lo depois desse tempo, e traz de volta o ecrã de bloqueio de *Para abrir o PlayGuard* quando o PlayGuard regressa após 5 min ou mais fora de foco.
- É verificado na camada de serviço, por isso nenhuma alteração lhe escapa; voltar a bloquear nunca o pede. *Mostrar o PIN* pede-o sempre, em todos os modos e mesmo logo depois de ter sido introduzido, e um `config.json` em falta ou danificado conta como *Antes de uma alteração*.
#### O PIN põe o controlo parental em pausa enquanto está no PlayGuard

- Só com *Segurança › Pausar o controlo parental enquanto estou no PlayGuard* ativado (desativado por predefinição: então introduzir o PIN deixa o controlo parental como está, e o tempo passado no PlayGuard conta como tempo de jogo). Ativá-lo pede o PIN.
- Depois de introduzido o PIN (o ecrã de bloqueio de *Para abrir o PlayGuard*, a primeira alteração com *Antes de uma alteração*, o regresso a partir do ecrã da criança…), o PlayGuard desbloqueia temporariamente o controlo parental, para que o tempo passado nele não seja descontado do tempo de jogo da criança; *Aberto a partir de um jogo* sai de Hoje.
- A etiqueta do cabeçalho diz *Controlo parental em pausa*, a verde-azulado; tocar-lhe explica o que isso é, com *Reativar agora* (depois disso, sem nova pausa até o PIN ser introduzido de novo). Hoje mostra o dia tal como é para a criança, e o mosaico de Segurança diz *Controlo parental em pausa*, sem aviso.
- Volta a ativar-se sozinho assim que se sai do PlayGuard (menu HOME, suspensão) ou ele é fechado, também a partir do menu HOME (a saída espera por isso), e enquanto o ecrã da criança ou o ecrã de bloqueio está visível. De volta ao PlayGuard com o PIN ainda válido, entra de novo em pausa.
- Não a partir do álbum (o tempo não conta aí, e um jogo pode ficar aberto por trás), não no modo só de leitura, e nunca por cima de um desbloqueio feito com *Desbloquear temporariamente*, que fica como está.
- Registado como qualquer desbloqueio: se o PlayGuard parar sem voltar a bloquear, o seu próximo arranque bloqueia de novo, e um reinício da consola fá-lo sozinho. Depende de o tempo não contar enquanto está desbloqueado, algo deduzido mas ainda não medido ([abaixo](#o-que-conta-como-tempo-de-jogo)).
- **Bloquear o menu homebrew ›** abre *Consola › Forwarders*, onde é bloqueado depois das suas verificações (veja [Forwarders](#forwarders)).
- **Sair** com o controlo parental ainda desbloqueado pergunta se deve voltar a bloqueá-lo (*Reativar e sair* fecha também o jogo sobre o qual o PlayGuard foi aberto, voltando ao menu HOME em vez do hbmenu); se for fechado a partir do menu HOME, o PlayGuard volta a bloqueá-lo da próxima vez que arrancar, e um reinício da consola bloqueia-o sozinho. A pausa do próprio PlayGuard não pergunta nada: termina quando o PlayGuard fecha.

### Aplicação móvel

- Se a aplicação Controlo parental Nintendo Switch está associada, a sua última sincronização, e **desvincular** (senão a sua próxima sincronização substitui os limites definidos aqui: enquanto está associada, cada alteração que o PlayGuard pede para confirmar avisa disso).
- Quando o redirecionamento DNS do Atmosphère (`atmosphere/hosts`) desvia o servidor de controlo parental da Nintendo, a aplicação não consegue sincronizar: o PlayGuard indica então *sincronização bloqueada (DNS)* aqui e não menciona a aplicação em mais lado nenhum (nenhuma nota nas caixas de diálogo, nenhuma entrada no sino, nenhum passo nos primeiros passos, nenhuma etiqueta em *Hoje*, nenhuma ajuda para decifrar, e desvincular pergunta sem dizer o que a aplicação substituiria).
- Enquanto está associada e consegue sincronizar, *Antes de desvincular: decifre duas definições* abre a comparação do bloco (veja [CONTRIBUTING.md](../../CONTRIBUTING.md#no-code-needed), em inglês).

### Eliminar todo o controlo parental

Duas confirmações, irreversível; antes é guardada uma cópia de segurança das definições.

### Sem acesso?

No fim do separador, o módulo de recuperação opcional, para um PIN esquecido com um limite de 0 minutos. Ⓐ abre-o em *Consola › Módulos opcionais*, que o instala.

## Histórico — o que o PlayGuard alterou, e as cópias de segurança

*[Experimentar no simulador ›](https://guard.boreale.dev/simulator/?tab=history)*

**No topo**, em mosaicos, quantas alterações estão registadas e o dia da última cópia de segurança: uma marca de verificação verde-azulada, ou um aviso âmbar quando não há nenhuma ou nenhuma há 30 dias.

### Histórico de alterações

- As 500 alterações mais recentes feitas pelo PlayGuard (limites, nível de restrição, PIN, desbloqueios, desvinculação, o relógio, restauros, pausar ou retomar a contagem decrescente …) e o que ele viu mudar fora dele, agrupadas por dia (*Hoje*, *Ontem*, depois o dia e a data), com a hora e a origem; um limite alterado para um dia nomeia esse dia.
- Uma alteração **recusada** pela consola também é listada, a âmbar, com o erro da consola e o que foi tentado (nunca reposta: nada mudou).
- *Mostrar* mantém um tipo (tempo de jogo, restrições, segurança e aplicação, visto fora do PlayGuard, recusado pela consola); *Exportar o que é mostrado* escreve-o em `exports/` em CSV, JSON, XLSX ou PDF.
- Ⓐ numa alteração mostra-a e, para um valor, **repõe o anterior** — com o mesmo desbloqueio e o mesmo PIN que qualquer alteração, indicando se mudou entretanto.
- Cada alteração guarda a versão do PlayGuard que a fez (uma versão de desenvolvimento: indicada como tal, com o seu commit e, para a compilação de um pull request, o seu número); uma alteração registada por uma versão mais recente do PlayGuard aparece como *Outra alteração*.

### Cópias de segurança

- **Fazer uma cópia de segurança / restaurar as definições** no cartão SD: nível de restrição, definições personalizadas, modo VR, organismo de classificação, limites diários, o alarme "Tempo esgotado" (com as ações avançadas ativas), e o bloco em bruto do temporizador, para registo — nunca o PIN.
- Um restauro lista só o que mudaria (nada a mudar: diz isso, sem desbloqueio e sem escrever nada), e avisa quando a cópia de segurança tinha um alarme de hora de dormir ativo (não é reposto).
- Escolha quantas cópias de segurança manter.

## Consola — o relógio de rede, os jogos instalados, o que a consola executa

*[Experimentar no simulador ›](https://guard.boreale.dev/simulator/?tab=console)*

**No topo**, o que a consola executa em mosaicos, em *Sistema* (o firmware, o Atmosphère, o hekate quando está no cartão SD e o sys-patch quando está instalado; Ⓐ abre as suas notas da versão), depois as ferramentas em mosaicos, em *Ferramentas* (*Relógio de rede*, *Jogos instalados* e *Forwarders*; as duas primeiras mostram o que precisa de atenção no seu canto; depois *Acesso remoto* e *Módulos opcionais*), depois as linhas da consola, a começar pela compatibilidade (só uma marca de verificação verde-azulada quando o PlayGuard foi verificado neste firmware, senão um aviso e o que está errado).

### Relógio de rede

*Relógio de rede ›*, um ecrã próprio — o relógio de que o temporizador de jogo depende. Uma consola que nunca se liga aos servidores da Nintendo mantém este relógio impreciso, o que falseia o temporizador de jogo.

- **Relógios da consola e da rede**, fuso horário, precisão. Quando o redirecionamento DNS do Atmosphère (`atmosphere/hosts`) desvia o servidor de hora da Nintendo (`aauth-lp1.ndas.srv.nintendo.net`), a consola deixa de conseguir acertar sozinha o seu relógio de rede: este ecrã, a entrada do sino e o cartão de *Hoje* dizem-no e remetem para a medição abaixo.
- **Escolher um servidor NTP público** (≈ 50 incluídos, por região, ou o seu).
- **Medir** com 3 servidores (mediana; um aviso âmbar quando não concordam): uma caixa de diálogo mostra o resultado — quanto o relógio de rede está adiantado ou atrasado, em grande, a hora dos servidores ao lado da do relógio de rede, a resposta de cada servidor — e acerta o relógio de rede a partir daí. Uma medição continua utilizável durante 2 minutos, com uma contagem decrescente por baixo de *Acertar o relógio de rede*. A confirmação diz: mudar o relógio repõe a zero o tempo de jogo de hoje, por isso todo o limite volta a ficar disponível.
- **Uma verificação do relógio de rede ao arrancar**: uma mensagem breve quando está desacertado em mais de um minuto; nunca acerta o relógio.

#### Nenhum servidor responde

Numa consola sem Internet, *Usar o relógio da consola* é proposto na caixa de diálogo do resultado, e *Acertar o relógio de rede pelo relógio da consola* por baixo da medição (e no cartão do relógio de rede de *Hoje*, depois de a sua medição falhar), atrás da mesma confirmação.

- O PlayGuard não tem mais nada com que comparar o relógio da consola: a confirmação mostra a sua hora e pede-lhe que a confira primeiro com um telemóvel ou um relógio, e um relógio da consola anterior a 2026 (nunca acertado) é recusado.
- Com *Sincronizar relógio através da Internet* desativado, o relógio da consola fica onde está, e ativar a definição depois já não o faz saltar; com ela ativa, o relógio da consola já segue o relógio de rede, por isso a confirmação diz para a desativar e acertar a hora à mão se estiver errada.
- Só escreve o relógio, não o controlo parental, por isso não precisa de desbloqueio (o PIN de *Proteger o PlayGuard com o PIN* continua a aplicar-se); como qualquer mudança de relógio, repõe a zero o tempo de jogo de hoje.
- Ainda não experimentado numa consola.

### Jogos instalados

*Jogos instalados* (um mosaico em *Consola › Ferramentas*) encontra os jogos que o menu HOME não consegue iniciar ou mostrar — um ícone que fica sempre a carregar, um "!", uma moldura tracejada — com o que está errado e o que fazer, por baixo do ícone do jogo, e quantos jogos instalados têm um problema.

**O que encontra:** nome e ícone ilegíveis, o próprio jogo em falta por trás da sua atualização, já nada instalado, ficheiros em falta, dados danificados detetados pela consola, um firmware mais recente exigido pelo jogo ou só pela sua atualização, chaves demasiado recentes para o firmware, uma licença (ticket) em falta, a consola a pedir uma atualização que foi removida, o cartão SD ilegível, um arranque que a consola recusa.

**A lista tem uma secção por solução**, cada uma com a sua contagem e o que fazer, os jogos com o mesmo problema juntos (dois jogos com o mesmo nome distinguem-se pela versão ou pelo ID do título):

| Secção | O que contém |
|---|---|
| **Reparáveis aqui** | O que o PlayGuard repara |
| **Por verificar** | Dados de que a consola suspeita: a verificação diz se é preciso reinstalar o jogo |
| **Reinstalar** | Ficheiros desaparecidos ou inutilizáveis, ou dados encontrados danificados: remover e depois instalar de novo |
| **Firmware ou cartão SD** | O jogo está bem, a consola não |

- **Verificar os dados:** o que a verificação de dados encontrou fica com o jogo até ser instalada outra versão: danificados, passa para *Reinstalar* como *Danificado (verificado)*, com a data e o código da consola; intactos, sai da lista. *Verificar estes jogos* verifica toda a secção *Por verificar*, um jogo a seguir ao outro, numa só caixa de diálogo, com as suas capas por cima (*jogo 2 de 5*), e depois diz quantos estão danificados, intactos ou não puderam ser verificados, com a remoção dos danificados à mão.
- **Vários jogos de uma vez:** Ⓨ escolhe jogos (uma marca de verificação à frente); a partir do primeiro escolhido, uma linha acima das secções diz quantos, e A nela escolhe também uma secção inteira; depois, uma só ação para todos: verificar os seus dados, ou removê-los (a sua lista guardada primeiro, uma pergunta, um PIN).
- **Remover vários:** as capas afastam-se a flutuar à medida que cada jogo sai (um "!" coral num que fica), a barra com um segmento por jogo (até 20); no fim, quantos foram removidos e o espaço libertado, e a caixa de diálogo fecha-se sozinha. Se ficarem alguns: as suas capas rodeadas a coral, e *Tentar de novo* para eles.
- **Em *Reinstalar*,** a lista pode ser guardada (JSON e texto, na pasta das exportações: nome, ID do título, versão, problema), e todos podem ser removidos de uma vez depois de uma pergunta e de um PIN: a lista é guardada primeiro e depois os jogos são removidos, para que um instalador deixe de os contar como instalados.
- **As ações:** quando uma ação é a solução, é o primeiro botão da caixa de diálogo; seguem-se as outras: aceitar de novo a versão instalada (quando a consola pede uma atualização removida), remover só a atualização ou um conteúdo adicional, verificar se os dados estão danificados (a verificação da própria consola, com o seu progresso e Cancelar), ou remover o jogo.
- As versões são mostradas como o jogo as mostra (*versão 1.4.0*, *atualização n.º 4*); depois de uma reparação, uma mensagem breve diz se o jogo voltou a funcionar. Os dados guardados ficam sempre; cada alteração está no histórico de alterações.
- **Quando verifica:** a última verificação aparece logo; depois só os jogos cuja entrada mudou são verificados de novo (todos os jogos uma vez por dia, depois de uma atualização do firmware, ou com Ⓧ *Verificar de novo*), cada um mostrado assim que é verificado. Ⓧ *Verificar de novo* abre uma caixa de diálogo: as capas dos jogos folheadas, uma barra e a contagem; *Cancelar* para depois do jogo em curso, e o que foi verificado fica. Sem nenhum jogo ainda listado, as capas vão passando no cartão enquanto verifica.
- Vários jogos avariados ao mesmo tempo apontam para o cartão SD ou para um jogo instalado a partir da outra memória (sysMMC / emuMMC).
- Ⓡ mostra os jogos como **lista** ou como **capas** (ícone, nome e problema; a escolha é mantida), como faz *Todos os jogos*.

### Forwarders

*Forwarders ›*, um ecrã próprio, um mosaico em *Ferramentas* (também *Segurança › Bloquear o menu homebrew*) — um homebrew no menu HOME, com o seu ícone e o seu nome, iniciado a partir dali como um jogo (um forwarder de NRO, como os que o sphaira cria).

- **Um cartão no topo** diz em que ponto está o menu homebrew: bloqueado ou aberto, os seus caminhos de volta (o forwarder do PlayGuard, os menus protegidos, os que ainda nunca foram iniciados a partir do menu HOME), o que ainda o abre sem o PIN, e se o sysmodule de recuperação está instalado. Ⓐ nele lista o que fazer, cada coisa a um toque (o passo seguinte para um bloqueio, proteger ou eliminar um forwarder que o abre sem o PIN, a verificação, a folha de recuperação).
- **A lista por grupos:** o do próprio PlayGuard, os outros que criou (protegidos ou não, num só cartão: o ícone diz qual), os criados por outra aplicação, os que já não estão no menu HOME. Um protegido tem um cadeado no ícone (uma marca de verificação para um menu que já se viu funcionar através da sua porta), um criado aqui sem o PIN um cadeado aberto a âmbar.
- **Ⓨ** abre esta parte do guia (um código QR). Depois de eliminar ou esquecer um forwarder, ou de mudar as suas horas sem PIN, *Pedir o PIN* ou *Fechado quando o tempo de jogo de hoje acaba*, **ZR** repõe-no durante alguns segundos (instalado novamente com o mesmo ID de programa, ou a definição como estava).
- **PlayGuard** com um só toque: o seu próprio ícone e nome, com o forwarder a iniciar o `.nro` a partir do qual o PlayGuard foi aberto.
- **Outro homebrew**: qualquer `.nro` da pasta `/switch` do cartão SD (e uma pasta abaixo), com o nome pedido primeiro (o do próprio `.nro`, para manter ou alterar), depois confirmado com o que o menu HOME vai mostrar: o seu ícone, o seu nome e a sua editora. O ícone é o do `.nro`, convertido no JPEG de 256 × 256 que o menu HOME mostra quando tem outro tamanho ou é um PNG (um comando no seu lugar quando o `.nro` não tem nenhum), como faz o sphaira. Um caminho com um espaço é passado ao homebrew entre aspas.
- **Os forwarders do menu HOME**, cada um com o seu ícone: os que o PlayGuard criou (Ⓐ: detalhes, mudar o nome, mudar a editora, instalar novamente, eliminar) e os que outra aplicação criou (detalhes, eliminar). Ⓐ abre uma folha por secções (*Este forwarder*, *Ao iniciar*, *Proteção*, *Instalar e eliminar*) sob o seu ícone e o seu estado, cada linha com o seu ícone e o que está definido agora (as suas horas; um interruptor para *Pedir o PIN*, *Fechado quando o tempo de jogo de hoje acaba* e cada opção de arranque), *Eliminar* a vermelho; B ou um toque fora dela fecha-a. *Detalhes* lista as suas regras da mesma forma, e depois a editora, a versão, o `.nro` e o ID de programa; Ⓐ numa regra (ou na editora) altera-a, como na folha. Por baixo dos protegidos, uma linha diz o que significa cada ícone que lá aparece. Um cujo `.nro` desapareceu indica-o; um eliminado nas Definições da consola continua na lista até ser instalado novamente ou esquecido. Instalar novamente mantém o seu ID de programa, por isso a entrada do menu HOME é substituída, não duplicada; um forwarder criado por outra ferramenta para o mesmo `.nro` também é substituído.
- **Protegido com o PIN** (Ⓐ › *Proteger com o PIN*, para um criado pelo PlayGuard que não seja o seu): iniciado a partir do menu HOME, o forwarder abre primeiro uma pequena porta (`config/playguard/gate.nro`, algumas centenas de kilobytes que o PlayGuard escreve a partir dos seus próprios ficheiros), que mostra logo o ecrã do PIN da consola, e depois o homebrew quando o PIN está certo; B ali volta ao menu HOME. À volta do ecrã do PIN, o da própria porta: o ícone e o nome do homebrew, *Protected by PlayGuard*, e como correu (a iniciar, nas suas horas livres, PIN não introduzido), com o aspeto do ecrã de carregamento do PlayGuard, no seu idioma e tema (*Preferências*) (`make gate-preview` desenha-o num computador). Mantém o seu ID de programa e o seu ícone, e o histórico de alterações regista-o. A sua linha mostra as suas regras em ícones, um por cada regra ativa: uma chave para o PIN, um cadeado aberto para horas sem PIN, um quadrado (stop) para horas em que nunca abre, um temporizador para fechado quando o tempo de jogo de hoje acaba; *Detalhes* (Ⓐ) dá-as por extenso. É preciso um PIN na consola. *Deixar de pedir o PIN* pede-o primeiro, seja qual for a opção de *Proteger o PlayGuard com o PIN*, tal como eliminá-lo ou voltar a criá-lo sem o PIN (*Adicionar um forwarder* para o mesmo `.nro`). Só protege o ícone do menu HOME: o `.nro` continua a iniciar a partir do menu homebrew, a não ser que este esteja bloqueado (abaixo). Um forwarder protegido por um PlayGuard anterior inicia o próprio PlayGuard como porta, o que continua a funcionar; *Reinstalar todos* passa-o para a pequena porta.
  - **Horas sem PIN** (Ⓐ num protegido): todos os dias, de segunda a sexta ou ao fim de semana, de uma hora a outra (também depois da meia-noite), ou *De segunda a sexta, e o fim de semana à parte* (um intervalo para cada); *Detalhes* indica quando. Defini-las pede o PIN. Seguem o relógio da consola, que as Definições da consola podem alterar: assim que se afasta mais de 5 minutos do relógio de rede, a porta pede o PIN mesmo assim nessas horas (*Sem horas livres: o relógio da consola difere do relógio da rede*), e o mesmo quando o relógio de rede não pode ser lido para o verificar (*Sem horas livres: relógio da rede indisponível*); *Consola › Relógio de rede* acerta os dois, também numa consola sem Internet.
  - **Horas em que nunca abre** (Ⓐ num protegido), definidas da mesma forma: a porta diz então *Não a esta hora* e as horas, e volta ao menu HOME, mesmo com o PIN introduzido (a hora de dormir para o homebrew). Têm prioridade sobre as horas sem PIN. Com o relógio da consola a mais de 5 minutos do relógio de rede, fica fechado quando qualquer um dos dois relógios está nessas horas, e uma pausa após recusas conta pelo relógio de rede e pelo relógio contínuo da consola (que nenhuma definição altera), por isso mudar a hora não as contorna.
  - **Fechado quando o tempo de jogo de hoje acabar** (Ⓐ num protegido): quando o temporizador de jogo da consola já não tem tempo hoje, a porta diz *O tempo de jogo de hoje acabou* e volta ao menu HOME, com ou sem PIN. Desativado, a não ser que seja definido.
  - **As regras estão no forwarder, não no cartão SD:** as horas, o limite do dia e a pausa após recusas vão para os dados do próprio forwarder (no armazenamento da consola), por isso um computador não pode acrescentar horas livres editando um ficheiro; alterar uma refaz o forwarder, em poucos segundos. O forwarder guarda também o SHA-256 da porta (`config/playguard/gate-<hash>.nro`): uma porta alterada no cartão SD não é iniciada (erro 2347-0100, [resolução de problemas](troubleshooting.md#um-forwarder-protegido-mostra-o-erro-2347-0100)). Um protegido por um PlayGuard anterior lê as suas regras do cartão: as suas horas sem PIN não são então respeitadas, e o sino propõe *Refazer*.
  - **Uma pausa após PIN recusados** (*Pausa após PIN recusados*, em *Menu homebrew*): cinco recusas seguidas (B no ecrã do PIN) e o forwarder deixa de o pedir durante 15 minutos, com a porta a dizer até quando; 3, 5 ou 10 tentativas e 15 minutos ou uma hora, ou nunca. O sino diz que forwarder está em pausa, e a sua linha também (um ícone de pausa, e até quando); *Levantar a pausa* (Ⓐ nele, com o PIN pedido) fá-lo pedir de novo de imediato. Cada pausa e cada pausa levantada fica no histórico de alterações.
  - **Cada tentativa fica no histórico de alterações**: aberto com o PIN, aberto nas suas horas, ou recusado (B no ecrã do PIN), à hora em que aconteceu; as tentativas de um tipo, num dia, para um forwarder formam uma só entrada, com as suas horas. A porta regista-as no cartão SD; o PlayGuard lê-as no arranque seguinte. *Histórico › Mostrar › Homebrew e forwarders* lista só estas, os forwarders e o bloqueio.
  - **Tentativas da semana** (em *Histórico* e em *Menu homebrew*): nos últimos sete dias, as aberturas de cada forwarder, as suas recusas e em que dias, e as vezes em que não abriu; guardadas durante quatro semanas. Quando as recusas desta semana são mais do dobro das da semana anterior (pelo menos cinco), o sino avisa.
  - **O ecrã da porta diz mais:** as suas horas sem PIN enquanto pede o PIN, e as recusas do dia até ao momento (a âmbar) enquanto o pede e depois de recusado, por baixo de *Peça a um adulto para introduzir o PIN*. Depois de *Protected by PlayGuard*, um ícone por cada regra que transporta, como a sua linha no PlayGuard: um cadeado aberto (horas sem PIN), um quadrado (horas em que nunca abre), um temporizador (o tempo de jogo de hoje). Depois de recusado, Ⓐ tenta de novo e Ⓑ volta ao menu HOME (tal como esperar 15 segundos), até começar a pausa após as recusas. Também diz até quando abre sem o PIN, quando acaba uma pausa (*daqui a 12 min*), o tempo de jogo que resta hoje (com *Fechado quando o tempo de jogo de hoje acaba*), e o código de erro quando o ecrã do PIN não consegue abrir. A moldura do ícone toma a cor do resultado. Se a porta alguma vez parar a caminho do homebrew, termina ao fim de 10 segundos, de volta ao menu HOME, em vez de deixar a consola no seu ecrã.
  - **O ecrã de abertura** (*Ecrã de abertura*, em *Menu homebrew*): iniciada a partir do menu HOME, a porta abre primeiro o homebrew da forma como o PlayGuard se abre a si próprio: centrado no azul-noite com o mostrador do Cadran à direita, o ícone do homebrew sobre um suave halo verde-azulado, o seu nome, *Protected by PlayGuard*, uma barra a encher e, em baixo, a marca do PlayGuard com o seu anel a rodar. Depois o ecrã do PIN da consola, e o homebrew logo que o PIN está certo (nas suas horas livres, logo a seguir à barra). Dura 2 segundos, a não ser que se escolha 1, 3 ou 5 segundos, ou *Desativado* (logo o ecrã do PIN); Ⓐ ou Ⓑ saltam o resto, e tentar de novo após uma recusa não o mostra outra vez. É lido do cartão SD a cada arranque (`config/playguard/gate_wait.txt`), por isso alterá-lo não volta a criar os forwarders.
- **O seu ícone a partir do álbum** (Ⓐ › *Mudar o ícone*): as 30 capturas mais recentes do álbum numa grelha, cada uma como o ícone que daria, primeiro o do `.nro` e assinalado o que está em uso; ao lado, o ícone que o menu HOME mostra agora e o que vai mostrar, o jogo em que a captura foi tirada e quando (*Hoje*, *Ontem*, ao segundo quando duas foram tiradas no mesmo minuto). L e R movem o quadrado do ícone ao longo da captura (cinco posições, da margem esquerda à direita), Y volta ao do `.nro`, A escolhe o selecionado: passa a ser o JPEG de 256 × 256 do menu HOME. Instalá-lo novamente mantém-no, com o enquadramento.
- **Ícones e idiomas:** um ícone de outro tamanho, um PNG, ou um JPEG com dados EXIF ou codificação progressiva (o menu HOME mostra-os como um "?") passa a ser um JPEG de 256 × 256 simples. Os dados de controlo do forwarder só declaram o inglês americano, o idioma do seu único ícone: assim, o ícone aparece numa consola configurada em qualquer idioma.
- **Ao iniciar** (Ⓐ num forwarder, como as opções de forwarder do sphaira): *Escolher um utilizador ao iniciar* (desativado por predefinição) pergunta que utilizador joga quando ele inicia, como faz um jogo, por isso o seu tempo de jogo é o desse utilizador em *Atividade* (o perfil do próprio adulto mantém as estatísticas da criança como estão); o limite de tempo de jogo da própria consola continua a contá-lo, seja quem for que jogue, como conta todos os jogos. *Capturas de ecrã* e *Captura de vídeo* (ativados por predefinição) deixam o botão de captura tirá-las. Cada interruptor pede o PIN e cria de novo o forwarder (o mesmo ID de programa), e fica no histórico de alterações; *Detalhes* lista-os.
- **Proteger todos os forwarders com o PIN** e **Reinstalar todos** (o ícone e a versão de um novo `.nro`, a porta tal como este PlayGuard a traz, um eliminado do menu HOME reposto): com um toque cada.
- **Bloquear o menu homebrew** (*Menu homebrew*, no fim do ecrã): nem o álbum nem um jogo iniciado com R premido abrem mais o menu homebrew, por isso um homebrew só inicia a partir do seu forwarder, que pode pedir o PIN.
  - **Passo a passo:** enquanto não puder ser bloqueado, Ⓐ lista os passos, numerados, o seguinte a âmbar e a um toque (*Passo seguinte*): um PIN, os patches de assinatura, o forwarder do PlayGuard, um menu homebrew protegido com o PIN (o sphaira encontrado em `/switch`, senão o hbmenu, criado já protegido de uma só vez), iniciado uma vez a partir do menu HOME; a lista volta depois de cada passo que cria um forwarder.
  - São precisos dois caminhos de volta para o PlayGuard a partir do menu HOME: o seu próprio forwarder, e um forwarder de um menu homebrew (sphaira ou hbmenu, pelo nome do `.nro`) protegido com o PIN, que mantém todos os homebrew, uma loja de homebrew incluída, à distância de um PIN. E um PIN na consola, e os patches de assinatura de que os forwarders precisam. Até lá, Ⓐ diz o que falta.
  - O caminho de volta pelo menu conta assim que se viu a funcionar: iniciado uma vez a partir do menu HOME, com o PIN introduzido na sua porta (o registo da porta di-lo, lido quando o PlayGuard abre ou bloqueia). Um menu protegido e nunca iniciado dessa forma aparece como tal.
  - Bloquear mostra as verificações numa lista (uma marca de verificação verde-azulada para o que está pronto, um triângulo âmbar ou coral para o que não está), e pede Ⓐ premido 2 segundos em *Bloquear* (ou um dedo mantido nele), com uma barra a encher entretanto; se soltar antes, nada muda. Depois vem a **folha de recuperação** (também escrita em `config/playguard/rescue.html`, uma página para abrir e imprimir num computador, no idioma da interface, com o código QR): como o anular a partir de um computador, ou com o sysmodule de recuperação quando está instalado, e um código QR para a ajuda ([abaixo](troubleshooting.md#menu-homebrew-bloqueado)); *Verificar o menu homebrew* também a tem.
  - No emuMMC (ou num sysMMC com um emuMMC configurado), avisa que o bloqueio está no cartão SD, por isso os dois sistemas o leem, enquanto os forwarders são de cada sistema: o outro sistema não tem nenhum destes caminhos de volta.
  - Enquanto está bloqueado, eliminar o forwarder do próprio PlayGuard, ou o último menu protegido (ou retirar-lhe o PIN), é recusado: desbloqueie primeiro. Eliminar um de vários menus protegidos pede confirmação, a coral, nomeando os que ficam.
  - Antes de bloquear, propõe proteger de uma só vez os forwarders criados pelo PlayGuard que ainda não estão protegidos, e diz quais continuariam a iniciar homebrew sem o PIN (um menu não protegido, ou um criado por outra aplicação), quais escrevem no cartão SD (um gestor de ficheiros, um instalador como o Cyberfoil ou o Goldleaf, um servidor FTP: poderiam alterar o ficheiro do bloqueio), e quando o uLaunch está instalado (inicia os homebrew à sua maneira, que o bloqueio não alcança).
  - O PIN recusado três vezes ou mais num dia na porta de um forwarder é assinalado no sino (que forwarder, o dia, quantas vezes).
  - Quando a escrita de um ficheiro falha, o erro diz porquê quando o consegue saber: o cartão SD cheio, ou nada pode ser escrito nele (o seu interruptor de bloqueio, ou o bit de arquivo que *Tools* do hekate corrige).
  - *Verificar o menu homebrew* mostra o que o Atmosphère vai ler (se o álbum e um jogo iniciado com R abrem o menu homebrew), os caminhos de volta, os patches de assinatura, a versão do Atmosphère com que foi bloqueado e o sysmodule de recuperação; o relatório de diagnóstico tem o mesmo. Se não estiver bloqueado, di-lo primeiro e propõe *Bloquear*, os passos que ainda faltam (a folha de recuperação mantém-se); um sysmodule de recuperação desatualizado, o PlayGuard com que veio e como substituí-lo (*Atualizar o módulo* quando esta compilação o traz). No emuMMC, ou no sysMMC com uma emuMMC configurada, diz que o outro sistema também lê o bloqueio, sem os forwarders instalados aqui.
  - São algumas linhas no fim de `atmosphere/config/override_config.ini`, entre dois comentários do PlayGuard; o resto do ficheiro fica como está, e é guardado tal como estava em `config/playguard/override_config.ini.backup`. O Atmosphère lê o ficheiro em cada início de um programa: não é preciso reiniciar. Desbloquear retira essas linhas e volta a escrever os valores do próprio Atmosphère para as chaves que o resto do ficheiro não define (o Atmosphère mantém o que leu até a consola reiniciar).
  - Bloquear e desbloquear pedem o PIN seja qual for a opção de *Proteger o PlayGuard com o PIN*, e ficam no histórico de alterações. Enquanto está bloqueado, o PlayGuard verifica em cada arranque que os dois caminhos de volta continuam lá; o sino avisa quando um desapareceu. Um bloqueio retirado do ficheiro fora do PlayGuard (um computador, um homebrew) é assinalado no sino e no histórico de alterações; o PlayGuard nunca o repõe sozinho, já que eliminar o ficheiro é o caminho de volta a partir de um computador. Eliminar um forwarder (Definições da consola › Gestão de dados, onde não é pedido nenhum PIN) nunca levanta o bloqueio: só retira um caminho de volta.
  - A partir de um computador: elimine `atmosphere/config/override_config.ini` (ou reponha a cópia de segurança) e depois reinicie a consola.
  - Com o sysmodule de recuperação instalado (opcional, [sysmodule/README.md](../../sysmodule/README.md), em inglês), um pedido `RESCUE` também o levanta no arranque, tal como uma atualização do Atmosphère desde que foi bloqueado (os patches de assinatura podem atrasar-se, e nenhum forwarder inicia sem eles): o arranque seguinte do PlayGuard indica-o e propõe voltar a bloqueá-lo.
- **Um forwarder eliminado fora do PlayGuard** (nas Definições da consola): assinalado uma vez no histórico de alterações, e no sino enquanto durar, com *Instalar novamente* ou *Esquecer* na sua linha.
- **Patches de assinatura:** a consola só inicia um forwarder com patches de assinatura do fs e do loader (sys-patch, ou ficheiros sigpatch), com todos os padrões que o sys-patch tem para este firmware aplicados. Quando faltam, ou o sys-patch deixou um de fora (um sys-patch mais antigo num firmware mais recente), o ecrã indica-o por cima da lista, dizendo o que não tem patch, e adicionar, criar o do próprio PlayGuard ou reinstalar todos avisa primeiro que o forwarder ainda não iniciaria: adicioná-lo mesmo assim (inicia assim que os patches estiverem instalados), ou cancelar. Bloquear o menu homebrew também espera por eles (*Consola › Patches de jogos* tem os detalhes). Os ficheiros sigpatch deixados ao lado do sys-patch são assinalados, e *Consola › Patches de jogos* põe-nos de parte (movidos, nunca eliminados). Cada forwarder mantém o loader com que foi criado: os criados por um PlayGuard mais antigo são propostos para serem criados de novo (o sino, *Consola › Forwarders*). O `.nro` fica onde está; eliminar um forwarder nunca lhe toca.
- **Tempo de jogo:** iniciado a partir do seu ícone, um homebrew corre como um jogo: o tempo de jogo conta-o e suspende-o como qualquer jogo quando o tempo acaba, e *Atividade* lista-o. O PlayGuard continua à mão a partir do menu de homebrew.
- **Nas cópias de segurança das definições** (*Histórico › Cópias de segurança*): o que o PlayGuard criou e se o menu homebrew estava bloqueado. Restaurar uma cópia de segurança propõe, depois do resto, repor os forwarders que já não estão no menu HOME (protegidos como estavam, com as suas horas e o seu ícone); um bloqueio é apenas indicado, para voltar a fazer a partir daqui.
- Cada alteração pede o PIN quando *Proteger o PlayGuard com o PIN* o pede, é recusada no modo só de leitura e fica no histórico de alterações. O que o PlayGuard criou é guardado por sistema (sysMMC / emuMMC), em `forwarders.json`.

### Acesso remoto

*Acesso remoto ›*, um mosaico em *Ferramentas* — uma ligação opcional a um broker MQTT e ao Home Assistant na sua rede local, desativada por predefinição ([o guia do Home Assistant](../home-assistant.md), em inglês). O broker e a sua versão MQTT, o nome da consola, o que fazer com as ordens (perguntar na consola, executá-las, recusá-las), se podem alterar o temporizador de jogo (desativado por predefinição), a descoberta do Home Assistant, as informações da própria consola (modelo, versões, armazenamento, bateria, rede, jogos; o número de série só com *Publicar o número de série*), códigos QR que instalam a integração PlayGuard a partir do HACS e adicionam a consola (depois de instalada, a sua versão e a sua versão mais recente), o estado em direto, *Sincronizar agora* e o registo.

### Módulos opcionais

*Módulos opcionais ›*, um mosaico em *Ferramentas* — instalar, atualizar, iniciar com a consola ou remover o módulo de recuperação e o agente da ligação remota, cada um no seu separador, a partir das cópias que o PlayGuard traz (sem precisar de computador). Cada alteração pede o PIN quando *Proteger o PlayGuard com o PIN* o pede.

### As linhas da consola

- **Firmware, Atmosphère, hekate** (quando o seu `bootloader/update.bin` está no cartão SD) **e sys-patch** (quando está instalado; a sua versão vem do seu `config/sys-patch/log.ini`), cada um num mosaico com a versão instalada, e depois a versão mais recente que a última procura de atualizações encontrou numa etiqueta âmbar (*23.0.1 disponível*, o mosaico contornado a âmbar, e quantas versões de atraso por baixo quando é mais de uma), ou *Atualizado*; um aviso no canto do mosaico do firmware quando o firmware mais recente precisa primeiro de um Atmosphère ou hekate mais recente, ou ainda não é suportado por eles (as notas da versão do firmware dizem porquê: as notas deles dizem que firmware suportam).
- **Notas da versão:** Ⓐ em cada um abre-as: um separador para a versão instalada, um para cada versão mais recente (cinco no máximo) e versões mais antigas nos separadores que sobram (seis no total; Ⓛ / Ⓡ passam de um para outro), abrindo na seguinte, com etiquetas (uma marca de verificação para a versão instalada, um brilho para uma mais recente, o firmware mais recente que suporta, correções de segurança, controlo parental alterado), as notas tal como publicadas (em inglês), *Ver online* (um código QR) e quando foram obtidas. As do Atmosphère, do hekate e do sys-patch vêm das suas versões no GitHub (as do sys-patch de `impeeza/sys-patch`), as do firmware do switchbrew; ficam guardadas no cartão SD para leitura offline e são obtidas de novo uma vez por dia (Ⓧ já).
- **Compatibilidade**, depois **armazenamento** (emuMMC ou sysMMC, com um ícone de informação quando o outro sistema tem o seu próprio controlo parental: Ⓐ explica).
- Se o Atmosphère **oculta o número de série** (*Sim (XAW00000000000)*, o número que o sistema vê então; no emuMMC, quando não oculta, um ícone de aviso, que Ⓐ explica), e depois o **número de série real** da consola, com o meio mascarado: quando é ocultado, lido da cópia PRODINFO que o Atmosphère guarda em `atmosphere/automatic_backups/` (a desta consola, pelo seu ID de dispositivo, quando o cartão SD tem várias). Ⓐ no seu ícone de informação mostra-o inteiro e de onde vem.
- **Patches de jogos** (sys-patch ou ficheiros sigpatch, recomendando o sys-patch quando só são usados ficheiros: um ícone de informação, ou de aviso quando algo precisa de ser revisto, que Ⓐ explica).

## Preferências

*[Experimentar no simulador ›](https://guard.boreale.dev/simulator/?tab=preferences)*

- O que configura o próprio PlayGuard:
  - Primeiro em mosaicos: idioma (todos os idiomas da consola, ou o da própria consola), tema (escuro por predefinição, claro, ou o da consola), tamanho do texto (automático / normal / maior) e o separador em que abre.
  - **A introdução** ao arrancar: o ecrã de carregamento (o seu ícone com o anel a rodar, a barra, o mostrador do Cadran à direita) durante 2 segundos (ativa por predefinição; Ⓐ ou Ⓑ salta-a; desativada, o PlayGuard abre assim que estiver pronto).
- As mensagens que mostra:
  - Os tipos de notificações (veja [acima](#como-se-comportam-as-entradas)).
  - Um **lembrete mensal para apoiar o PlayGuard** (ativo por predefinição, nunca no primeiro mês nem logo depois de uma atualização; *Não mostrar novamente* no lembrete, ou este interruptor, desativa-o de vez, incluindo nas atualizações).
- Como se atualiza: *Procurar atualizações todos os dias* ao arrancar, e *Atualizar com* (veja [Sobre](#versão-e-atualizações)).
- **Primeiros passos** de novo (também em *Sobre › Ajuda e diagnóstico*).
- Uma definição que só diz respeito a uma funcionalidade fica junto dela: o tempo extra em *Tempo de jogo*, reativar o controlo parental em *Segurança*, a verificação do relógio em *Consola › Relógio de rede*. O histórico de alterações e as cópias de segurança têm o seu separador, [Histórico](#histórico--o-que-o-playguard-alterou-e-as-cópias-de-segurança); procurar atualizações agora e *Ajuda e diagnóstico* estão em *Sobre*.

## Sobre

*[Experimentar no simulador ›](https://guard.boreale.dev/simulator/?tab=about)*

### Versão e atualizações

- No topo, o seu cartão: o símbolo, o nome, a versão.
- O PlayGuard instalado: versão (a âmbar quando há uma mais recente; Ⓐ abre as suas **notas da versão**).
- **Procurar atualizações** agora; *Procurar atualizações todos os dias* ao arrancar está em *Preferências*.
- *Atualizar com* (em *Preferências*): no PlayGuard, que transfere a versão, verifica-a, substitui-se a si próprio e reinicia, ou sphaira, Homebrew App Store ou manualmente (perguntado na primeira atualização quando nada foi escolhido). Passar a vez a uma loja pede o PIN como uma alteração (*Proteger o PlayGuard com o PIN › Antes de uma alteração*), e sempre enquanto o menu homebrew estiver bloqueado: uma loja inicia qualquer homebrew. Instalado a partir da Homebrew App Store ou da App Store do sphaira, a loja é a forma recomendada aí, para que acompanhe a versão; quando o seu catálogo ainda não tem a nova versão (publica-a depois de uma revisão, muitas vezes alguns dias mais tarde), o PlayGuard indica-o e propõe atualizar-se diretamente.
- Enquanto se conhece uma versão mais recente, o PlayGuard abre em **O PlayGuard X.Y.Z está disponível** (a sua data e notas): *Atualizar*, ou *Mais tarde* durante 1 dia, 1 semana ou 30 dias no máximo.
- As notas da versão são as do próprio PlayGuard, no mesmo ecrã que as do Atmosphère e do hekate em *Consola*: a partir das suas versões no GitHub, um separador por versão (a instalada, as mais recentes, as mais antigas), cada uma com os seus destaques em cartões e todas as alterações, guardadas no cartão SD para leitura offline (as da versão instalada também estão na aplicação, para quando ainda nada foi obtido).
- Depois de uma atualização, o PlayGuard abre uma vez em **Novidades da X.Y.Z** (os mesmos destaques e depois os códigos QR).

### Ajuda e diagnóstico

*Ajuda e diagnóstico ›*, um ecrã próprio, por baixo de *Procurar atualizações*:

- **No topo**, em *Para começar*, dois mosaicos: *Primeiros passos* e *Enviar um relatório…*; depois *Diagnóstico*.
- **Primeiros passos** abre de novo o guia (com um passo *Desvincular a aplicação móvel* enquanto está associada, um passo *Reativar o alarme "Tempo esgotado"* enquanto está desativado, um passo *Proteger o PlayGuard* depois de definido um PIN — a escolha *Proteger o PlayGuard com o PIN* —, e um interruptor para deixar de aparecer ao arrancar). Por baixo de *Fechar*, *Apoiar o PlayGuard* mostra os códigos QR de apoio.
- Como o PlayGuard foi iniciado (uma aplicação sobre um jogo ou a partir do seu ícone no menu HOME, ou um applet) e a sua pasta de dados.
- **Exportar relatório de diagnóstico**, ou **enviá-lo online** (*Enviar um relatório…*; veja [Comunicar um erro](troubleshooting.md#comunicar-um-erro)); quantos relatórios guardados, ficheiros de falha, registos de sessão e ligações de relatórios enviados online são mantidos, por número, idade ou tamanho.

### Créditos

*Créditos e licença ›*: quem o fez, aquilo em que se baseia, a sua licença e o seu código-fonte num código QR, numa só caixa de diálogo. Depois, como **apoiar o PlayGuard** ([GitHub Sponsors](https://github.com/sponsors/JigSawFr), [Ko-fi](https://ko-fi.com/jigsawfr), mostrados como códigos QR a ler com um telemóvel), e um pequeno *Made in France* 🇫🇷.

## Segurança desde a conceção

**Escrever o limite de tempo de jogo em segurança.** Substituir a configuração do temporizador enquanto ele conta desestabiliza o Atmosphère. Por isso, o PlayGuard verifica primeiro o estado; se o temporizador estiver ativo, a confirmação diz que o controlo parental vai ser desbloqueado temporariamente — com o PIN guardado, **não precisa de se lembrar dele**. Um toque desbloqueia, verifica que o sistema indica mesmo o desbloqueio, escreve e **volta logo a bloquear** (ou propõe fazê-lo, se *Reativar o controlo parental após uma alteração* estiver desativado). A camada de serviço volta a verificar o estado mesmo antes de escrever, por isso nenhum ecrã pode contornar esta proteção.

> [!CAUTION]
> Um limite abaixo do tempo já jogado hoje suspende o jogo assim que o controlo parental volta a ser bloqueado. A confirmação avisa quando isso aconteceria.

**Acabaram-se as falhas na 22.5.** O `pctl:a`, o serviço privilegiado de controlo parental, aceita **uma única sessão**. As versões antigas da aplicação original mantinham-na aberta, por isso o pedido de PIN do menu HOME (ou o applet de PIN) não a conseguia obter e o Atmosphère podia falhar. O PlayGuard abre a sessão para cada ação e liberta-a de imediato; as atualizações periódicas param enquanto a aplicação está em segundo plano. *(Diagnóstico do [fork de anbingxi](https://github.com/anbingxi/NX-Pctl-Manager/tree/diag/fw22-5-readonly).)*

**Nada corre em segundo plano.** O limite, o PIN, os avisos e a suspensão são os da própria consola; o PlayGuard só muda as suas definições. A ligação remota opcional funciona enquanto o PlayGuard está aberto, ou sempre depois de instalado o seu agente opcional, e executa uma ordem exatamente como uma alteração feita na consola (o mesmo desbloqueio, o mesmo novo bloqueio, o mesmo modo só de leitura e o mesmo histórico); nunca pode eliminar o controlo parental, desvincular a aplicação móvel nem mexer no PIN.

## O que conta como tempo de jogo

O temporizador de jogo é o da consola: o PlayGuard lê-o, não conta nada sozinho. O que se sabe, e com que certeza ([parental-controls.md](../parental-controls.md#time-spent-and-time-left), em inglês, tem as medições):

| Situação | Conta? | Com que certeza |
|---|---|---|
| Um jogo a correr | Sim | Medido (22.0.0) |
| **Um jogo deixado aberto no menu HOME** (suspenso em segundo plano) | **Sim**: feche-o, ou ponha a consola em modo de suspensão | Relatado por pais ([Arqade](https://gaming.stackexchange.com/questions/398954/)); bate certo com as medições (a consola conta enquanto uma aplicação está aberta) |
| Modo de suspensão | Não | Relatado ([Arqade](https://gaming.stackexchange.com/questions/398954/)); não medido pelo PlayGuard |
| **PlayGuard aberto sobre um jogo** (como aplicação) | **Sim**, como esse jogo | Medido (22.0.0): abra-o com o utilizador de um adulto, ou a partir do álbum para uma consulta rápida |
| **PlayGuard iniciado a partir de um forwarder** (o seu ícone no menu HOME, também uma aplicação) | **Sim**, com o nome do ícone | Deduzido: uma aplicação como qualquer outra; não medido pelo PlayGuard |
| PlayGuard aberto a partir do álbum (modo applet) | Não | Deduzido: não é uma aplicação, e o tempo fora das aplicações não pareceu contar |
| Enquanto o controlo parental está desbloqueado temporariamente (também a pausa do PlayGuard enquanto está nele) | Talvez não | Não medido: questão em aberto |
| Depois de o relógio ser mudado | O dia recomeça; depois de um salto para trás, a contagem para até o relógio passar a hora antiga | Observado (22.0.0); *Hoje* indica quando a consola não está a contar |
