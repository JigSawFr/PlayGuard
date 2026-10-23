<!-- translated from docs/troubleshooting.md, sha256 0f51172fd98a -->
# Solução de problemas

Como voltar a ter acesso a um console bloqueado, os problemas comuns, relatar um bug e o modo de desenvolvedor. Volte ao [README](../../README.md).

## Console bloqueado?

O PlayGuard só vê o controle parental do sistema em que roda: **o emuMMC e o sysMMC têm cada um o seu** (um PIN removido em um continua no outro). Abra-o em cada um que precisar de correção.

### Console usado

Você sabe o PIN, mas o app do celular do dono anterior continua vinculado (a desvinculação falha, ou a restauração de fábrica pede a conta dele).

*Segurança › Desvincular o app do celular* e depois, se você não quiser nenhum controle parental, *Excluir todo o controle dos pais*. Os dois funcionam offline, tanto no emuMMC quanto no sysMMC.

### PIN esquecido

*Mostrar o PIN* pede o próprio PIN, então não ajuda aqui. Com o sysmodule de recuperação instalado (abaixo), coloque um arquivo `RESCUE` e reinicie: a tela de recuperação então define um novo PIN (ela nunca mostra o antigo). Sem ele, *Excluir todo o controle dos pais* recomeça do zero (um backup das configurações é salvo antes; ele nunca contém o PIN): com *Proteger o PlayGuard com o PIN* ativado (o padrão, *Antes de alterar*), coloque antes o cartão SD em um computador e mude `"pin_lock"` para `"off"` em `sd:/config/playguard/config.json`.

### O temporizador bloqueia tudo (limite de 0 minuto) e o PIN foi esquecido

Nesse caso, o próprio PlayGuard não consegue abrir. Instale **com antecedência** o sysmodule de recuperação opcional (`playguard-rescue.zip`); quando estiver bloqueado, coloque um arquivo vazio `config/playguard/RESCUE` no cartão SD e ligue o console — ele desbloqueia o console para que o PlayGuard possa abrir. Veja [`sysmodule/README.md`](../../sysmodule/README.md) (em inglês).

O módulo precisa acompanhar a versão do PlayGuard: a cada atualização do PlayGuard, extraia de novo o `playguard-rescue.zip` dessa versão (ou *Atualizar* na entrada *O sysmodule de recuperação está desatualizado* do sino de notificações) e depois reinicie o console. Um módulo mais antigo deixa um relatório que o PlayGuard não consegue verificar: a tela de recuperação então não oferece nada, diz que o módulo é mais antigo, e repetir o `RESCUE` dá o mesmo resultado até ele ser atualizado.

### Menu homebrew bloqueado

*Console › Forwarders › Bloquear o menu homebrew* impede que o álbum e um jogo aberto com R abram o menu homebrew. Se o forwarder do PlayGuard e o menu protegido tiverem sumido os dois (excluídos nas Configurações do console, ou no outro sistema: o bloqueio fica no cartão SD, lido tanto pelo sysMMC quanto pelo emuMMC):

- **Por um computador:** exclua `atmosphere/config/override_config.ini` no cartão SD (ou recoloque `config/playguard/override_config.ini.backup`, o arquivo como era) e depois reinicie o console.
- **Com o sysmodule de recuperação instalado** (opcional): coloque um arquivo vazio `config/playguard/RESCUE` no cartão SD e reinicie; o sysmodule desfaz o bloqueio na inicialização, e o PlayGuard avisa isso na próxima abertura. Uma atualização do Atmosphère desde o bloqueio o desfaz do mesmo jeito.

### Console não desbloqueado

O PlayGuard não pode ajudar: ele precisa do Atmosphère. O procedimento da chave mestra do suporte da Nintendo é o caminho oficial.

## Problemas comuns

### O tempo restante não diminui

*Hoje* diz *O console não está contando o tempo de jogo agora* quando ele não muda por cerca de 90 s. As causas prováveis:

- **O relógio de rede nunca foi ajustado.** *Console › Relógio de rede › Medir a diferença de horário* e depois ajuste-o ([relógio de rede](features.md#relógio-de-rede)).
- **O relógio foi atrasado.** Depois de um salto para trás, a contagem para até o relógio passar do horário antigo; não há nada a fazer.
- **Um limite acabou de ser gravado.** Abra um jogo por um minuto e depois confira de novo.
- **O controle parental está desbloqueado temporariamente.** Nesse caso, o tempo de jogo talvez não seja contado: *Hoje › Reativar* (ou um toque na etiqueta do cabeçalho).

### Meus limites mudaram sozinhos

- **O app do celular continua vinculado.** A próxima sincronização dele substitui o que o PlayGuard definiu: *Segurança › Desvincular o app do celular*.
- **Outra ferramenta, ou um perfil.** *Hoje* mostra um cartão âmbar para limites diferentes dos que o PlayGuard viu por último, e *Histórico › Alterações do PlayGuard* lista cada mudança vista fora do PlayGuard. Ⓐ em uma delas restaura o valor anterior.
- **Um perfil agendado** aplicado, ou desfeito quando o período dele terminou (*Tempo de jogo › Perfis › Perfil agendado*).

### A criança jogou além do limite

- **O console só avisa.** *Tempo de jogo › Quando o tempo acabar*: escolha *Suspender o software*.
- **O controle parental ficou desbloqueado.** Enquanto ele estiver desbloqueado, o limite de hoje fica em pausa: a etiqueta do cabeçalho avisa isso em todas as abas, e um toque nela leva a *Hoje › Reativar*. Reiniciar o console também o bloqueia.
- **O relógio foi atrasado.** A contagem para até o relógio passar do horário antigo ([o que conta](features.md#o-que-conta-como-tempo-de-jogo)).
- **O outro sistema.** O emuMMC e o sysMMC têm cada um seu próprio controle parental: configure-o nos dois, ou esconda o menu de inicialização do hekate ([compatibilidade](compatibility.md#emummc-e-sysmmc)).

### O limite inteiro está disponível de novo hoje

Mudar o relógio (o relógio de rede ou o relógio do console) recomeça o tempo de jogo de hoje. Ajuste o relógio antes de a criança jogar, não depois.

### O PlayGuard abre somente leitura

O console roda um firmware mais recente que o mais recente testado: o PlayGuard verifica se uma versão mais nova oferece suporte a ele e oferece a atualização ([firmware mais recente](compatibility.md#firmware-mais-recente)). Um lápis riscado à direita do título indica que o app está somente leitura; um toque nele abre a tela de firmware.

### O PlayGuard pede o PIN o tempo todo, ou nunca

*Segurança › Proteger o PlayGuard com o PIN*: *Antes de alterar* (o padrão) pede uma vez, antes da primeira mudança, e o PIN vale até o PlayGuard fechar. *Pedir o PIN de novo após 5 minutos* pede de novo depois desse tempo. *Mostrar o PIN* sempre pede.

### Um jogo não abre, ou mostra um "!" no menu HOME

*Console › Jogos instalados* diz o que está errado com cada um e o que fazer; os dados salvos sempre ficam ([jogos instalados](features.md#jogos-instalados)).

### Um forwarder protegido mostra o erro 2347-0100

O portão dele (`config/playguard/gate-….nro`) não é mais o arquivo com que o forwarder foi feito: ele foi alterado ou substituído no cartão SD, então o forwarder não o abre. Abra o PlayGuard pelo forwarder dele próprio: ele grava o portão de novo ao iniciar; *Console › Forwarders › Reinstalar todos* refaz todos os forwarders com ele.

### Um forwarder protegido fecha na hora ("ocorreu um erro")

Antes de qualquer tela do PlayGuard, sem código de erro (o relatório do Atmosphère em `atmosphere/crash_reports/` cita o módulo `forwarder`, resultado `2000-0000`): ele foi feito por um PlayGuard cujo portão foi compilado sem os recursos dele, que o loader do forwarder recusou. Atualize o PlayGuard e depois crie o forwarder de novo: o sino e *Console › Forwarders* oferecem isso para cada forwarder feito com um loader mais antigo.

### Um forwarder protegido fica na tela do portão depois do PIN

Com o PIN aceito, a tela com o ícone do homebrew continua lá e nada mais acontece: o portão desenhou essa tela com um único buffer e ficou esperando para sempre para desenhar a próxima. Atualize o PlayGuard e depois crie o forwarder de novo: o sino e *Console › Forwarders* oferecem isso para cada forwarder feito com um portão mais antigo.

### Um forwarder mostra "Não foi possível iniciar o software"

O console se recusou a abri-lo: ainda há uma verificação de assinatura ativa. Na ordem em que vale a pena olhar:

- **Arquivos de sigpatch junto do sys-patch.** Arquivos `.ips` em `atmosphere/kip_patches/fs_patches`, `loader_patches` ou `atmosphere/exefs_patches/es_patches` enquanto o sys-patch roda: duas ferramentas aplicando patches nas mesmas verificações, a causa mais citada nos tópicos sobre sys-patch e sigpatches do GBAtemp. *Console › Patches de jogos* avisa disso, e Ⓐ ali os coloca de lado (movidos para `config/playguard/sigpatches-aside/`, nunca excluídos; o `bootloader/patches.ini` do hekate continua). Reinicie o console.
- **Um padrão que o sys-patch não aplicou.** Os forwarders precisam dos patches de fs e de loader dele, todos os padrões que ele tem para este firmware (`noncasigchk_17.0.0+`, `nocntchk_19.0.0+`, `noacidsigchk_10.0.0+`), não só um por módulo. *Sobre › Ajuda e diagnóstico › Exportar relatório de diagnóstico* lista, na linha `game patches`, a versão do sys-patch e cada padrão que ficou de fora; `sd:/config/sys-patch/log.ini` traz o mesmo. Confira se `config/sys-patch/config.ini` aplica patches neste sistema (`patch_emummc=1` no emuMMC) e não desativa nenhum deles; o próprio mantenedor do sys-patch diz que a 23.0.x não precisa de um sys-patch mais novo que o 1.6.2.3.
- **Nenhuma reinicialização desde que os patches mudaram.** O sys-patch aplica os patches na inicialização.
- **Um forwarder feito por um PlayGuard mais antigo.** O sino e *Console › Forwarders* oferecem criar de novo os que foram compilados com um loader mais antigo.

### O sino ou uma aba mostra um selo âmbar

Algo precisa de você: Ⓨ abre a lista, e Ⓐ em uma entrada aplica a solução ou abre onde ela é feita ([notificações](features.md#notificações--o-que-precisa-de-você-em-qualquer-aba)).

## Relatar um bug

Um erro mostra o código do sistema (como `0x0001088E`) com o que se sabe sobre ele: o que fazer, para os códigos cujo significado está documentado; caso contrário, um relatório ajuda.

1. *Sobre › Ajuda e diagnóstico › Exportar relatório de diagnóstico* salva um arquivo de texto em `sd:/config/playguard/logs/reports/`: firmware, versão do Atmosphère, quanto tempo a abertura levou e quanto tempo levaram as telas lentas (o histórico de alterações, *Todos os jogos*, a verificação dos jogos instalados, as abas que se atualizam sozinhas), relógios, armazenamento, estado do mascaramento do número de série e dos patches de jogos, o que o redirecionamento de DNS do Atmosphère faz com os servidores de controle parental e de hora da Nintendo (lido do arquivo hosts dele no cartão SD, não um teste de rede, com o final do `atmosphere/logs/dns_mitm_startup.log` do próprio Atmosphère), o resultado bruto de cada consulta ao controle parental, e os logs: como a última execução terminou, a memória e o espaço livre no cartão SD, os arquivos de falha mais recentes e as últimas 80 linhas registradas. **Ele nunca contém o PIN nem o número de série.**
2. [Abra uma issue](https://github.com/JigSawFr/PlayGuard/issues/new) (em inglês) e anexe-o.

### Enviar um relatório online

Com o console online, *Sobre › Ajuda e diagnóstico › Enviar um relatório…* (Ⓧ no relatório de diagnóstico, no modo de desenvolvedor) envia o relatório para um gist secreto na sua conta do GitHub, depois de dizer exatamente o que vai ser enviado. Um relatório salvo antes em `logs/reports/` pode ser enviado do mesmo jeito.

- **O que é enviado:** o relatório e cada arquivo como um arquivo separado:
  - os relatórios de falha mais recentes: os 3 últimos do PlayGuard em `logs/crashes/`, e os 2 últimos do Atmosphère de um homebrew na última semana, em `sd:/atmosphere/crash_reports/` (a falha de outro homebrew pode estar entre eles: a lista mostrada antes do envio diz quais);
  - os logs desta execução e da anterior (`logs/sessions/`; de um longo: o começo e os últimos 64 KiB);
  - os arquivos de depuração: a referência do bloco do temporizador, o fim da gravação do temporizador enquanto *Registrar o limite de tempo de jogo* estiver ativado, as 20 últimas alterações do histórico (sem nomes de jogos, contas ou perfis, e só com a data), e as configurações do PlayGuard.
- **Login:** sem login, o PlayGuard oferece entrar primeiro; *Conta do GitHub*, logo abaixo depois do login, sai da conta; quando o GitHub recusa o gist, o PlayGuard oferece entrar de novo.
- **Depois:** ele mostra dois QR codes, com o link embaixo do primeiro e o ID dele em grupos de quatro para ler em voz alta ou digitar: o relatório, para quem já está ajudando você, e o formulário de relato de bug já preenchido com o link, as suas versões, como o PlayGuard foi aberto (por cima de um jogo, pelo ícone dele no menu HOME ou pelo Álbum; acrescente o menu) e o sistema (sysMMC ou emuMMC), para escanear e relatar um bug. O link também fica nas notificações, e cada um é guardado em `logs/uploads/` como um atalho de Internet (`.url`) que um computador abre no navegador.

> [!WARNING]
> Qualquer pessoa com o link pode ler o relatório, até você apagar o gist no GitHub, e um relato de bug aberto com ele é público: a confirmação avisa.

### O que o PlayGuard guarda no cartão SD

Tudo fica em `sd:/config/playguard/logs/`, em texto simples que um computador abre:

- `sessions/session_<date>.txt` — **um arquivo por execução**: tudo o que o PlayGuard registrou (o que antes só chegava a um PC pelo nxlink), cada linha com seu horário e nível (`E` erro, `W` aviso, `I` informação), e uma última linha dizendo como a execução terminou: `=== end: clean exit ===` (saída normal), ou `=== end: crash, see logs/crashes/… ===` (falha). Um arquivo para de crescer em 1 MiB.
- `crashes/crash_<date>.txt` — **um arquivo por falha**, com a primeira linha dizendo o que aconteceu:
  - uma **exceção da CPU** (um ponteiro inválido, uma instrução indefinida, um estouro de pilha): o tipo de falha e o endereço acessado, o backtrace e os registradores. Os endereços são deslocamentos dentro do `playguard.nro` (`playguard+0x1a2b3c`): o mapa do linker da compilação (o artefato `-linker-map` da execução de CI dela) indica a função. Depois, o PlayGuard termina como antes: a tela de erro do console, e o relatório do próprio Atmosphère em `sd:/atmosphere/crash_reports/`;
  - uma **exceção não tratada**: o tipo e a mensagem dela, e o backtrace;
  - uma **execução que não terminou**: detectada na abertura seguinte, quando o arquivo de sessão da última execução não tem linha de fim (uma falha que o PlayGuard não conseguiu capturar, um fechamento pelo menu HOME enquanto ele estava travado, ou o console desligado), com as últimas 60 linhas dessa execução.

  Os arquivos de falha capturados enquanto o PlayGuard roda terminam com as últimas 80 linhas registradas antes da falha.
- `reports/`, `uploads/`, `play_timer/` — os relatórios salvos, os links dos relatórios enviados online, a gravação do temporizador.

*Sobre › Ajuda e diagnóstico* define quantos relatórios, arquivos de falha, logs de sessão e links são mantidos (cada pasta separadamente; o mais recente sempre fica).

## Modo de desenvolvedor (investigar um novo firmware)

Pressione Ⓨ sete vezes em *Sobre › Versão*; as ferramentas de desenvolvedor aparecem no fim de *Sobre › Ajuda e diagnóstico*. Ele acrescenta:

- uma chave **somente leitura** — com ela ativada, o app não pode alterar nada: o jeito seguro de investigar um novo firmware (a tela de firmware oferece isso diretamente);
- o relatório de diagnóstico na tela (Ⓨ salva, Ⓧ envia online), e um atalho para exportá-lo na aba Tempo de jogo;
- **Autoteste do console**: o que o PlayGuard verifica sozinho neste console antes que [a lista de verificação do console](../hardware-test.md) (em inglês) seja percorrida à mão — o PIN, os patches de assinatura, a pasta de dados, cada forwarder criado pelo PlayGuard (no menu HOME, protegido, visto funcionando pelo portão dele), o portão no cartão SD, o que o Atmosphère lê do bloqueio do menu homebrew, os caminhos de volta, o sysmodule de recuperação. Ele não altera nada, e grava o resultado em Markdown em `config/playguard/self-test.md`, para colar na pull request;
- **Instalar outra compilação** no lugar da atual, para testar uma correção antes de ela ser publicada ([abaixo](#instalar-outra-compilação));
- **Comparar bloco do tempo de jogo** (só enquanto o app do celular estiver vinculado e puder sincronizar — não com um arquivo hosts bloqueando a Nintendo; também em *Segurança*), para decifrar configurações que o PlayGuard ainda não mostra: salve o bloco bruto como referência, mude uma configuração no app do celular, volte — o PlayGuard lista os valores que mudaram (salvos em `logs/reports/` se você pedir). Horários de dormir diferentes por dia, definidos pelo app do celular, poderiam ser confirmados assim. Anexe esse arquivo a uma issue, ou envie-o com o relatório (*Enviar um relatório…*).
- **Registrar o limite de tempo de jogo**: a cada 30 s enquanto o PlayGuard estiver aberto, uma linha com o que o temporizador informa (tempo restante, tempo gasto, o bloco bruto de configurações…) em `logs/play_timer/log.csv`, um arquivo pronto para planilha, mais uma linha logo depois de cada alteração feita pelo PlayGuard, dizendo qual (a última coluna, `event`), e uma nota quando o relógio andou mais do que o tempo que passou. Deixado aberto depois da meia-noite, ou até o tempo acabar, ele mostra o que um único relatório não mostra: quando o tempo gasto é zerado, o que o console diz perto do fim. A chave é lembrada; ela só grava no modo de desenvolvedor. O que já se sabe está em [parental-controls.md](../parental-controls.md) (em inglês).
- **Limpar o cache** e **Excluir os arquivos de log**, cada um depois de uma confirmação que diz quantos arquivos e quanto espaço: o cache (`cache/` e o `nand/<system>/cache/` de cada sistema: nomes e ícones dos jogos, os últimos números da Atividade, a lista de compilações) é lido de novo do console quando necessário; os arquivos de log são os arquivos de falha, os logs de sessão (menos o desta execução, ainda sendo gravado), os relatórios salvos e a gravação do temporizador, enquanto `uploads/` (os links de remoção dos relatórios enviados online) e a referência do bloco ficam. Configurações, histórico, backups, perfis e exportações nunca são tocados.
- **Executar as migrações de novo**: as mudanças das organizações anteriores — a pasta de dados ao lado do `.nro`, os logs de antes das subpastas deles, o que todos os sistemas compartilhavam — cada uma em uma linha para marcar ou desmarcar (todas marcadas no início; A alterna uma), depois *Executar as N escolhidas* e uma confirmação que lista cada uma com o que ela move. Elas rodam na hora, não importa o que o `migrations.json` registre (senão, cada uma roda só uma vez, na primeira abertura de uma compilação que precise dela; o nível registrado nunca diminui); um arquivo que já está no lugar nunca é substituído. Uma caixa de diálogo diz quantos itens foram movidos; cada etapa fica no log de sessão.
- **Estatísticas de imagem**: uma linha por segundo em `logs/frame_stats.txt`, com o tempo que a tela levou para ser desenhada e as imagens que chegaram atrasadas, para comparar uma tela com outra no console. **Halo de fundo** desliga a luz suave atrás das telas (o modo da linha passa então a terminar em `-noglow`), para medir quanto ela custa.

### Instalar outra compilação

*Instalar outra compilação* coloca uma compilação no lugar do próprio `.nro` do PlayGuard, para testar uma correção antes de ela ser publicada.

**O que ela lista**, cada tipo em uma aba própria (Ⓛ / Ⓡ, mostrados dos dois lados das abas, passam de uma para a outra; a tela abre na aba da compilação instalada, e cada aba tem, depois do nome, um brilho e o número de compilações mais novas que a instalada):

- uma das 20 últimas versões publicadas (pré-lançamentos incluídos, identificados);
- um dos 20 últimos commits de `main`;
- a compilação mais recente de uma pull request aberta a partir de um branch deste repositório (nunca de um fork: o código dele ainda não foi revisado). A, na aba das pull requests, quando aparece, inverte a ordem (compilação mais recente primeiro, ou por número): a dica do A diz qual.

**Cada compilação** mostra o título (o assunto de um commit ou de uma pull request; a mensagem completa de um commit é a descrição) acima de etiquetas: o tipo de mudança (*Novidade*, *Correção*, *Docs*…) e o escopo, *Incompatível*, a pull request de onde veio um commit, como terminaram as verificações (*CI OK*, *CI falhou*, *CI em andamento*), pré-lançamento, uma marca de seleção quando é a compilação instalada ou um brilho quando é mais nova; a idade (em minutos, horas e depois dias) e o commit à direita. Uma compilação abre primeiro a página dela: etiquetas (o tipo, as verificações, a marca de seleção ou o brilho), um cartão de dados (data da compilação, tamanho, commit, checksum, quando o GitHub vai apagá-la), **Instalar**, **Ver no GitHub** (a página dela como QR code, para um celular), um aviso quando as verificações dela ainda estão em andamento, depois as notas de uma versão ou a descrição de uma pull request, montadas a partir do Markdown do GitHub (títulos, listas, caixas de tarefa, código, citações e alertas, tabelas).

**Leitura segura:** tudo o que vem do GitHub é mostrado como texto simples: nenhum link é aberto, nenhum HTML ou script é executado, e os caracteres invisíveis que poderiam disfarçar um título (inversões da direita para a esquerda, caracteres de largura zero, caracteres de controle, UTF-8 inválido) são removidos.

**Login.** As versões publicadas não precisam de nada; as outras são artefatos do workflow de build, que o GitHub só entrega a usuários com login. **Conta do GitHub** faz o login com um código e um QR code para escanear com um celular. As únicas permissões pedidas são ler os arquivos do workflow de build (Actions) e criar gists, para *Enviar um relatório…*; fora isso, o token só pode ler o que é público. Ele é guardado em `github_token`, nunca é enviado com um relatório, e *Conta do GitHub* sai da conta. Com login feito, Ⓨ (*Buscar*) recebe o número de uma pull request (aberta ou fechada) ou um commit (de 7 a 40 dígitos hexadecimais) e encontra a compilação, somente de um branch deste repositório.

**Instalação:** o PlayGuard baixa a compilação, verifica (tamanho, o SHA-256 registrado pelo GitHub, o cabeçalho NRO), coloca-a no lugar do próprio `.nro` e reinicia com ela (o PIN é pedido antes sempre que houver um). A janela dela diz em que etapa está, o tamanho e a velocidade do download, e qual compilação ela substitui; Cancelar some assim que a compilação começa a ser colocada no lugar.

- A lista é guardada por 10 minutos e mostrada na hora, com a idade dela (Ⓧ busca de novo, e a tela também, a cada 3 minutos, enquanto as verificações de uma compilação estão em andamento); enquanto ela é buscada, uma barra à direita do cabeçalho mostra o progresso. O que a instalação faz é dito na parte de baixo da tela.
- Um download interrompido (rede perdida) continua de onde parou na próxima vez que a mesma compilação for instalada.
- Os artefatos expiram depois de 90 dias.

**Voltar atrás.** A compilação substituída é guardada (`playguard.nro.previous`): **Voltar para a compilação anterior**, na mesma lista, troca as duas de novo, depois do PIN; ela identifica essa compilação pela versão, pela pull request e pelo commit. Se uma compilação instalada por aqui (ou pela atualização dentro do próprio PlayGuard) falhar antes da primeira tela, a abertura seguinte do PlayGuard oferece voltar para a compilação anterior em uma tela de texto simples, antes de carregar qualquer outra coisa (A volta, B abre mesmo assim). Voltar pede o PIN na própria tela de PIN do console, quando há um definido (não na tela de bloqueio do PlayGuard, que pode ser o que está falhando); sem ele, a compilação instalada abre. A mesma lista volta para qualquer versão publicada a qualquer momento.

**Qual compilação está rodando:** *Sobre › Versão* mostra o commit no modo de desenvolvedor. O cabeçalho de cada tela mostra a versão em execução ao lado do título: a de uma versão publicada em uma etiqueta cinza (`1.2.0`), a de qualquer outra compilação em uma etiqueta âmbar seguida do commit, para que uma compilação de teste nunca seja confundida com a versão publicada.
