<!-- translated from docs/troubleshooting.md, sha256 0f51172fd98a -->
# Resolução de problemas

Voltar a entrar numa consola bloqueada, os problemas comuns, comunicar um erro e o modo de programador. Voltar ao [README](../../README.md).

## Consola bloqueada?

O PlayGuard só vê o controlo parental do sistema em que corre: **o emuMMC e o sysMMC têm cada um o seu** (um PIN removido num continua lá no outro). Execute-o em cada um que precise de ser corrigido.

### Consola em segunda mão

Sabe o PIN, mas a aplicação móvel do antigo dono continua associada (a desvinculação falha, ou uma reposição de fábrica pede a conta dele).

*Segurança › Desvincular a aplicação móvel* e depois, se não quiser controlo parental nenhum, *Eliminar todo o controlo parental*. Ambos funcionam offline, tanto no emuMMC como no sysMMC.

### PIN esquecido

*Mostrar o PIN* pede o próprio PIN, por isso não ajuda aqui. Com o sysmodule de recuperação instalado (abaixo), coloque um ficheiro `RESCUE` e reinicie: o ecrã de recuperação define então um novo (nunca mostra o antigo). Sem ele, *Eliminar todo o controlo parental* recomeça do zero (antes é guardada uma cópia de segurança das definições; nunca contém o PIN): com *Proteger o PlayGuard com o PIN* ativo (a predefinição, *Antes de uma alteração*), ponha primeiro o cartão SD num computador e defina `"pin_lock"` como `"off"` em `sd:/config/playguard/config.json`.

### O temporizador de jogo bloqueia tudo (limite de 0 minutos) e o PIN foi esquecido

Nesse caso, o próprio PlayGuard não consegue arrancar. Instale **antecipadamente** o sysmodule de recuperação opcional (`playguard-rescue.zip`); quando ficar bloqueado, coloque um ficheiro vazio `config/playguard/RESCUE` no cartão SD e arranque — ele desbloqueia a consola para que o PlayGuard possa abrir. Veja [`sysmodule/README.md`](../../sysmodule/README.md) (em inglês).

O módulo tem de acompanhar a versão do PlayGuard: a cada atualização do PlayGuard, extraia de novo o `playguard-rescue.zip` dessa versão (ou *Atualizar* na entrada *O sysmodule de recuperação está desatualizado* do sino de notificações) e depois reinicie a consola. Um módulo mais antigo deixa um relatório que o PlayGuard não consegue verificar: o ecrã de recuperação não propõe então nada, diz que o módulo é mais antigo, e voltar a fazer `RESCUE` dá o mesmo resultado até ele ser atualizado.

### Menu homebrew bloqueado

*Consola › Forwarders › Bloquear o menu homebrew* impede que o álbum e um jogo iniciado com R abram o menu homebrew. Se o forwarder do PlayGuard e o menu protegido tiverem ambos desaparecido (eliminados nas Definições da consola, ou no outro sistema: o bloqueio está no cartão SD, lido tanto pelo sysMMC como pelo emuMMC):

- **A partir de um computador:** elimine `atmosphere/config/override_config.ini` no cartão SD (ou reponha `config/playguard/override_config.ini.backup`, o ficheiro tal como estava) e depois reinicie a consola.
- **Com o sysmodule de recuperação instalado** (opcional): coloque um ficheiro vazio `config/playguard/RESCUE` no cartão SD e reinicie; o sysmodule levanta o bloqueio no arranque, e o PlayGuard indica-o no arranque seguinte. Uma atualização do Atmosphère desde que foi bloqueado levanta-o da mesma forma.

### Consola não modificada

O PlayGuard não pode ajudar: precisa do Atmosphère. O procedimento da chave mestra do apoio ao cliente da Nintendo é a via oficial.

## Problemas comuns

### O tempo restante não desce

*Hoje* indica *A consola não está a contar o tempo de jogo neste momento* quando ele não se mexe durante cerca de 90 s. As causas prováveis:

- **O relógio de rede nunca foi acertado.** *Consola › Relógio de rede › Medir a diferença horária*, e depois acerte-o ([relógio de rede](features.md#relógio-de-rede)).
- **O relógio foi atrasado.** Depois de um salto para trás, a contagem para até o relógio passar a hora antiga; não há nada a fazer.
- **Acabou de ser escrito um limite.** Inicie um jogo durante um minuto e verifique de novo.
- **O controlo parental está desbloqueado temporariamente.** O tempo de jogo pode então não ser contado: *Hoje › Reativar* (ou um toque na etiqueta do cabeçalho).

### Os meus limites mudaram sozinhos

- **A aplicação móvel continua associada.** A sua próxima sincronização substitui o que o PlayGuard definiu: *Segurança › Desvincular a aplicação móvel*.
- **Outra ferramenta, ou um perfil.** *Hoje* mostra um cartão âmbar para limites diferentes dos que o PlayGuard viu da última vez, e *Histórico › Alterações do PlayGuard* lista cada alteração vista fora do PlayGuard. Ⓐ numa delas repõe o valor anterior.
- **Um perfil agendado** posto em vigor, ou reposto quando o seu período terminou (*Tempo de jogo › Perfis › Perfil agendado*).

### A criança jogou para além do limite

- **A consola só avisa.** *Tempo de jogo › Quando o tempo acabar*: escolha *Suspender o software*.
- **O controlo parental ficou desbloqueado.** Enquanto está, o limite de hoje fica em pausa: a etiqueta do cabeçalho indica-o em todos os separadores, e um toque nela leva a *Hoje › Reativar*. Um reinício da consola também o bloqueia.
- **O relógio foi atrasado.** A contagem para até o relógio passar a hora antiga ([o que conta](features.md#o-que-conta-como-tempo-de-jogo)).
- **O outro sistema.** O emuMMC e o sysMMC têm cada um o seu próprio controlo parental: defina-o nos dois, ou esconda o menu de arranque do hekate ([compatibilidade](compatibility.md#emummc-e-sysmmc)).

### Todo o limite volta a estar disponível hoje

Mudar o relógio (o relógio de rede ou o relógio da consola) repõe a zero o tempo de jogo de hoje. Acerte o relógio antes de a criança jogar, não depois.

### O PlayGuard abre só de leitura

A consola tem um firmware mais recente do que o mais recente testado: o PlayGuard verifica se uma versão mais recente o suporta e propõe atualizar ([firmware mais recente](compatibility.md#firmware-mais-recente)). Um lápis riscado à direita do título indica que a aplicação está só de leitura; tocar-lhe abre o ecrã do firmware.

### O PlayGuard pede o PIN a toda a hora, ou nunca

*Segurança › Proteger o PlayGuard com o PIN*: *Antes de uma alteração* (a predefinição) pede-o uma vez, antes da primeira alteração, e o PIN vale até o PlayGuard fechar. *Pedir o PIN novamente após 5 minutos* volta a pedi-lo depois desse tempo. *Mostrar o PIN* pede-o sempre.

### Um jogo não arranca, ou mostra um "!" no menu HOME

*Consola › Jogos instalados* diz o que está errado com cada um e o que fazer; os dados guardados ficam sempre ([jogos instalados](features.md#jogos-instalados)).

### Um forwarder protegido mostra o erro 2347-0100

A sua porta (`config/playguard/gate-….nro`) já não é o ficheiro com que o forwarder foi criado: foi alterada ou substituída no cartão SD, por isso o forwarder não a inicia. Abra o PlayGuard a partir do seu próprio forwarder: ele volta a escrever a porta ao arrancar; *Consola › Forwarders › Reinstalar todos* refaz todos os forwarders com ela.

### Um forwarder protegido fecha logo ("ocorreu um erro")

Antes de qualquer ecrã do PlayGuard, sem código de erro (o relatório do Atmosphère em `atmosphere/crash_reports/` indica o módulo `forwarder`, resultado `2000-0000`): foi criado por um PlayGuard cuja porta foi compilada sem os seus recursos, que o loader do forwarder recusou. Atualize o PlayGuard e depois crie-o de novo: o sino e *Consola › Forwarders* propõem-no para cada forwarder criado com um loader mais antigo.

### Um forwarder protegido fica no ecrã da sua porta depois do PIN

O PIN aceite, o ecrã com o ícone do homebrew fica e nada mais acontece: a sua porta desenhou esse ecrã com um único buffer e ficou à espera para sempre para desenhar o seguinte. Atualize o PlayGuard e depois crie-o de novo: o sino e *Consola › Forwarders* propõem-no para cada forwarder criado com uma porta mais antiga.

### Um forwarder mostra "Não foi possível iniciar o software"

A consola recusou iniciá-lo: ainda há uma verificação de assinatura ativa. Pela ordem em que deve procurar:

- **Ficheiros sigpatch ao lado do sys-patch.** Ficheiros `.ips` em `atmosphere/kip_patches/fs_patches`, `loader_patches` ou `atmosphere/exefs_patches/es_patches` enquanto o sys-patch corre: duas ferramentas a aplicar patches às mesmas verificações, a causa que os tópicos sobre sys-patch e sigpatches no GBAtemp mais apontam. *Consola › Patches de jogos* avisa disso, e Ⓐ aí põe-nos de parte (movidos para `config/playguard/sigpatches-aside/`, nunca eliminados; o `bootloader/patches.ini` do hekate fica). Reinicie a consola.
- **Um padrão que o sys-patch não aplicou.** Os forwarders precisam dos seus patches do fs e do loader, todos os padrões que tem para este firmware (`noncasigchk_17.0.0+`, `nocntchk_19.0.0+`, `noacidsigchk_10.0.0+`), não só um por módulo. *Sobre › Ajuda e diagnóstico › Exportar relatório de diagnóstico* lista, na sua linha `game patches`, a versão do sys-patch e cada padrão deixado de fora; `sd:/config/sys-patch/log.ini` tem o mesmo. Verifique que `config/sys-patch/config.ini` aplica patches a este sistema (`patch_emummc=1` no emuMMC) e não desativa nenhum deles; o próprio responsável pelo sys-patch diz que a 23.0.x não precisa de um sys-patch mais recente do que o 1.6.2.3.
- **Nenhum reinício desde que os patches mudaram.** O sys-patch aplica os patches no arranque.
- **Um forwarder criado por um PlayGuard mais antigo.** O sino e *Consola › Forwarders* propõem criar de novo os que foram compilados com um loader mais antigo.

### O sino ou um separador mostra um distintivo âmbar

Há algo que precisa de si: Ⓨ abre a lista, e Ⓐ numa entrada faz a correção ou abre o sítio onde ela se faz ([notificações](features.md#notificações--o-que-precisa-de-si-a-partir-de-qualquer-separador)).

## Comunicar um erro

Um erro mostra o código do sistema (por exemplo `0x0001088E`) com o que se sabe sobre ele: o que fazer, para os códigos cujo significado está documentado; caso contrário, um relatório ajuda.

1. *Sobre › Ajuda e diagnóstico › Exportar relatório de diagnóstico* guarda um ficheiro de texto em `sd:/config/playguard/logs/reports/`: firmware, versão do Atmosphère, quanto tempo demorou o arranque e quanto demoraram os ecrãs lentos (o histórico de alterações, *Todos os jogos*, a verificação dos jogos instalados, os separadores que se atualizam sozinhos), relógios, armazenamento, estado da ocultação do número de série e dos patches de jogos, o que o redirecionamento DNS do Atmosphère faz com os servidores de controlo parental e de hora da Nintendo (lido do seu ficheiro hosts no cartão SD, não um teste de rede, com o fim do `atmosphere/logs/dns_mitm_startup.log` do próprio Atmosphère), o resultado em bruto de cada consulta ao controlo parental, e os registos: como terminou a última sessão, a memória e o espaço livre no cartão SD, os ficheiros de falha mais recentes e as últimas 80 linhas registadas. **Nunca contém o PIN nem o número de série.**
2. [Abra uma issue](https://github.com/JigSawFr/PlayGuard/issues/new) (em inglês ou francês) e anexe-o.

### Enviar um relatório online

Com a consola online, *Sobre › Ajuda e diagnóstico › Enviar um relatório…* (Ⓧ no relatório de diagnóstico, no modo de programador) envia-o para um gist secreto na sua conta GitHub, depois de dizer exatamente o que vai ser enviado. Um relatório guardado antes em `logs/reports/` pode ser enviado da mesma forma.

- **O que é enviado:** o relatório e cada ficheiro como um ficheiro próprio:
  - os relatórios de falha mais recentes: os 3 últimos do PlayGuard em `logs/crashes/`, e os 2 últimos do Atmosphère de um homebrew da última semana, em `sd:/atmosphere/crash_reports/` (a falha de outro homebrew pode estar entre eles: a lista mostrada antes do envio diz quais);
  - os registos desta sessão e da anterior (`logs/sessions/`; um longo: o seu início e os seus últimos 64 KiB);
  - os ficheiros de depuração: a referência do bloco do temporizador de jogo, o fim do registo do temporizador enquanto *Registar o temporizador de jogo* está ativo, as 20 últimas alterações do histórico de alterações (sem nomes de jogos, contas ou perfis, e só com a data), e as definições do PlayGuard.
- **Iniciar sessão:** sem sessão iniciada, o PlayGuard propõe iniciar sessão primeiro; *Conta GitHub*, logo abaixo depois de iniciar sessão, termina a sessão; quando o GitHub recusa o gist, o PlayGuard propõe iniciar sessão de novo.
- **Depois:** mostra dois códigos QR, com a ligação por baixo do primeiro, o seu identificador em grupos de quatro para ler em voz alta ou escrever: o relatório, para quem já o está a ajudar, e o formulário de comunicação de erros com a ligação, as suas versões, como o PlayGuard foi aberto (sobre um jogo, a partir do seu ícone no menu HOME ou a partir do Álbum; acrescente o menu) e o sistema (sysMMC ou emuMMC) já preenchidos, para digitalizar e comunicar um erro. A ligação fica também nas notificações, e cada uma é guardada em `logs/uploads/` como um atalho de Internet (`.url`) que um computador abre no navegador.

> [!WARNING]
> Qualquer pessoa com a ligação pode ler o relatório, até eliminar o gist no GitHub, e uma comunicação de erro aberta com ele é pública: a confirmação avisa.

### O que o PlayGuard guarda no cartão SD

Está tudo em `sd:/config/playguard/logs/`, texto simples que um computador abre:

- `sessions/session_<date>.txt` — **um ficheiro por sessão**: tudo o que o PlayGuard registou (o que antes só chegava a um PC através do nxlink), cada linha com a hora e o nível (`E` erro, `W` aviso, `I` informação), e uma última linha que diz como a sessão terminou: `=== end: clean exit ===` (saída normal), ou `=== end: crash, see logs/crashes/… ===` (falha). Um ficheiro deixa de crescer a 1 MiB.
- `crashes/crash_<date>.txt` — **um ficheiro por falha**, cuja primeira linha diz o que aconteceu:
  - uma **exceção do processador** (um ponteiro inválido, uma instrução indefinida, um transbordo da pilha): o tipo de falha e o endereço afetado, o backtrace e os registos do processador. Os endereços são deslocamentos dentro de `playguard.nro` (`playguard+0x1a2b3c`): o mapa do linker da compilação (o artefacto `-linker-map` da sua execução de CI) indica a função. O PlayGuard termina depois como antes: o ecrã de erro da consola, e o relatório do próprio Atmosphère em `sd:/atmosphere/crash_reports/`;
  - uma **exceção não tratada**: o seu tipo e mensagem, e o backtrace;
  - uma **sessão que não terminou**: detetada no arranque seguinte, quando o ficheiro de sessão da última sessão não tem linha de fim (uma falha que o PlayGuard não conseguiu apanhar, um fecho a partir do menu HOME enquanto estava bloqueado, ou a consola desligada), com as últimas 60 linhas dessa sessão.

  Os ficheiros de falha apanhados enquanto o PlayGuard corre terminam com as últimas 80 linhas registadas antes da falha.
- `reports/`, `uploads/`, `play_timer/` — os relatórios guardados, as ligações dos relatórios enviados online, o registo do temporizador de jogo.

*Sobre › Ajuda e diagnóstico* define quantos relatórios, ficheiros de falha, registos de sessão e ligações são mantidos (cada pasta por si; o mais recente fica sempre).

## Modo de programador (investigar um novo firmware)

Prima Ⓨ sete vezes em *Sobre › Versão*; as ferramentas de programador aparecem no fim de *Sobre › Ajuda e diagnóstico*. Acrescenta:

- um interruptor **só de leitura** — com ele ativo, a aplicação não pode alterar nada: a forma segura de investigar um novo firmware (o ecrã do firmware propõe-no diretamente);
- o relatório de diagnóstico no ecrã (Ⓨ guarda-o, Ⓧ envia-o online), e um atalho para o exportar a partir do separador Tempo de jogo;
- **Autoteste da consola**: o que o PlayGuard verifica sozinho nesta consola antes de [a lista de verificação na consola](../hardware-test.md) (em inglês) ser percorrida à mão — o PIN, os patches de assinatura, a pasta de dados, cada forwarder criado pelo PlayGuard (no menu HOME, protegido, visto a funcionar através da sua porta), a porta no cartão SD, o que o Atmosphère lê do bloqueio do menu homebrew, os caminhos de volta, o sysmodule de recuperação. Não altera nada, e escreve o resultado em Markdown em `config/playguard/self-test.md`, para colar no pull request;
- **Instalar outra compilação** no lugar da atual, para testar uma correção antes de ser publicada ([abaixo](#instalar-outra-compilação));
- **Comparar o bloco do temporizador** (só enquanto a aplicação móvel está associada e consegue sincronizar — não com um ficheiro hosts que bloqueie a Nintendo; também em *Segurança*), para decifrar definições que o PlayGuard ainda não mostra: guarde o bloco em bruto como referência, altere uma definição na aplicação móvel, volte — o PlayGuard lista os valores que mudaram (guardados em `logs/reports/` a pedido). Horas de dormir diferentes por dia, definidas na aplicação móvel, poderiam ser confirmadas assim. Anexe esse ficheiro a uma issue, ou envie-o com o relatório (*Enviar um relatório…*).
- **Registar o temporizador de jogo**: a cada 30 s enquanto o PlayGuard está aberto, uma linha com o que o temporizador de jogo indica (tempo restante, tempo gasto, o bloco de definições em bruto…) em `logs/play_timer/log.csv`, um ficheiro pronto para uma folha de cálculo, mais uma linha logo depois de cada alteração feita pelo PlayGuard, a dizer qual (a última coluna, `event`), e uma nota quando o relógio avançou mais do que o tempo que passou. Deixado aberto durante a meia-noite, ou até o tempo acabar, mostra o que um só relatório não consegue: quando o tempo gasto é reposto, o que a consola diz perto do fim. O interruptor é memorizado; só regista no modo de programador. O que já se sabe está em [parental-controls.md](../parental-controls.md) (em inglês).
- **Limpar a cache** e **Eliminar os ficheiros de registo**, cada um depois de uma confirmação que diz quantos ficheiros e quanto espaço: a cache (`cache/` e o `nand/<sistema>/cache/` de cada sistema: nomes e ícones dos jogos, os últimos números da Atividade, a lista de compilações) é lida de novo da consola quando for preciso; os ficheiros de registo são os ficheiros de falha, os registos de sessão (exceto o desta sessão, ainda a ser escrito), os relatórios guardados e o registo do temporizador de jogo, enquanto `uploads/` (as ligações de remoção dos relatórios enviados online) e a referência do bloco ficam. As definições, o histórico, as cópias de segurança, os perfis e as exportações nunca são tocados.
- **Executar de novo as migrações**: as mudanças das organizações anteriores — a pasta de dados ao lado do `.nro`, os registos de antes das suas subpastas, o que todos os sistemas partilhavam — cada uma numa linha para marcar ou desmarcar (todas marcadas de início; A alterna uma), depois *Executar as N escolhidas* e uma confirmação que as enumera com o que cada uma move. São executadas logo, diga o `migrations.json` o que disser (de outro modo, cada uma só é executada uma vez, no primeiro arranque de uma compilação que precise dela; o nível que regista nunca desce); um ficheiro já no lugar nunca é substituído. Uma janela indica quantos elementos foram movidos; cada passo fica no registo de sessão.
- **Estatísticas de imagem**: uma linha por segundo em `logs/frame_stats.txt`, com o tempo que o ecrã levou a ser desenhado e as imagens que chegaram atrasadas, para comparar um ecrã com outro na consola. **Halo de fundo** desliga a luz suave por trás dos ecrãs (o modo da linha passa então a terminar em `-noglow`), para medir o que ela custa.

### Instalar outra compilação

*Instalar outra compilação* coloca uma compilação no lugar do próprio `.nro` do PlayGuard, para testar uma correção antes de ser publicada.

**O que lista**, cada tipo num separador próprio (Ⓛ / Ⓡ, mostrados de cada lado dos separadores, passam de um para o seguinte; o ecrã abre no da compilação instalada, e cada separador tem, a seguir ao seu nome, um brilho e o número de compilações mais recentes do que a instalada):

- uma das 20 últimas versões publicadas (pré-versões incluídas, assinaladas);
- um dos 20 últimos commits do `main`;
- a compilação mais recente de um pull request aberto a partir de um ramo deste repositório (nunca de um fork: o seu código ainda não foi revisto). A no separador dos pull requests, quando é mostrado, ordena-os ao contrário (compilação mais recente primeiro, ou por número): a indicação de A diz qual.

**Cada compilação** mostra o seu título (o assunto de um commit ou de um pull request; a mensagem completa de um commit é a sua descrição) sobre etiquetas: o tipo de alteração (*Novidade*, *Correção*, *Docs*…) e o seu âmbito, *Incompatível*, o pull request de onde veio um commit, como terminaram as verificações (*CI OK*, *CI falhou*, *CI em curso*), pré-versão, uma marca de verificação quando é a compilação instalada ou um brilho quando é mais recente; a sua idade (em minutos, horas e depois dias) e o seu commit à direita. Uma compilação abre primeiro a sua página: etiquetas (o seu tipo, as suas verificações, a marca de verificação ou o brilho), um cartão de dados (compilada em, tamanho, commit, soma de verificação, quando o GitHub a elimina), **Instalar**, **Ver no GitHub** (a sua página como código QR para um telemóvel), um aviso quando as suas verificações ainda estão em curso, depois as notas de uma versão ou a descrição de um pull request, desenhadas a partir do Markdown do GitHub (títulos, listas, caixas de tarefas, código, citações e alertas, tabelas).

**Seguro de ler:** tudo o que vem do GitHub é mostrado como texto simples: nenhuma ligação é aberta, nenhum HTML ou script é executado, e os caracteres invisíveis que poderiam disfarçar um título (inversões da direita para a esquerda, caracteres de largura zero, caracteres de controlo, UTF-8 inválido) são removidos.

**Iniciar sessão.** As versões publicadas não precisam de nada; as outras são os artefactos do workflow de compilação, que o GitHub só entrega a utilizadores com sessão iniciada. **Conta GitHub** inicia sessão com um código e um código QR a ler com um telemóvel. As únicas permissões pedidas são ler os ficheiros do workflow de compilação (Actions) e criar gists, para *Enviar um relatório…*; de resto, o token só pode ler o que é público. É guardado em `github_token`, nunca enviado com um relatório, e *Conta GitHub* termina a sessão. Com sessão iniciada, Ⓨ (*Procurar*) aceita o número de um pull request (aberto ou fechado) ou um commit (7 a 40 dígitos hexadecimais) e encontra a sua compilação, apenas a partir de um ramo deste repositório.

**Instalar:** o PlayGuard transfere a compilação, verifica-a (tamanho, o SHA-256 que o GitHub regista, o cabeçalho NRO), coloca-a no lugar do seu próprio `.nro` e reinicia com ela (o PIN é pedido primeiro sempre que houver um definido). A sua janela diz em que passo está, o tamanho e a velocidade da transferência, e que compilação substitui; Cancelar desaparece assim que a compilação está a ser colocada no lugar.

- A lista é mantida durante 10 minutos e mostrada logo, com a sua idade (Ⓧ obtém-na de novo, e o ecrã também, a cada 3 minutos, enquanto as verificações de uma compilação estão em curso); enquanto é obtida, uma barra à direita do cabeçalho mostra o progresso. O que a instalação faz é dito no fundo do ecrã.
- Uma transferência interrompida (rede perdida) continua de onde parou da próxima vez que a mesma compilação for instalada.
- Os artefactos expiram ao fim de 90 dias.

**Voltar atrás.** A compilação substituída é mantida (`playguard.nro.previous`): **Voltar à compilação anterior**, na mesma lista, troca-as de novo, depois do PIN; identifica essa compilação pela sua versão, o seu pull request e o seu commit. Se uma compilação instalada a partir daqui (ou pela atualização no próprio PlayGuard, *Atualizar com › No PlayGuard*) falhar antes do primeiro ecrã, o arranque seguinte do PlayGuard propõe voltar à compilação anterior num ecrã de texto simples, antes de carregar qualquer outra coisa (A volta atrás, B arranca-a mesmo assim). Voltar atrás pede o PIN no próprio ecrã do PIN da consola, quando há um definido (não no ecrã de bloqueio do PlayGuard, que pode ser o que falha); sem ele, arranca a compilação instalada. A mesma lista volta a qualquer versão publicada a qualquer momento.

**Que compilação está a correr:** *Sobre › Versão* mostra o commit no modo de programador. O cabeçalho de cada ecrã mostra a versão em execução junto do título: a de uma versão publicada numa etiqueta cinzenta (`1.2.0`), a de qualquer outra compilação numa etiqueta âmbar seguida do seu commit, para que uma compilação de teste nunca seja confundida com a versão publicada.
