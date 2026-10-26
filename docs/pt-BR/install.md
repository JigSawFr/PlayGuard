<!-- translated from docs/install.md, sha256 900d9a183a17 -->
# Instalação

Como instalar, atualizar e remover o PlayGuard, e o que ele grava no cartão SD. Depois de instalado: [primeiros passos](getting-started.md). Volte ao [README](../../README.md).

## Onde obter

Escolha uma opção:

- **Homebrew App Store** / **App Store do sphaira** (mesmo catálogo) — Procure *PlayGuard* e instale-o. As novas versões são baixadas automaticamente, depois de uma revisão: muitas vezes alguns dias depois do GitHub. O PlayGuard sabe que foi instalado por ali e recomenda atualizar pela loja.
- **Manual** — Baixe o `playguard.zip` da [versão mais recente](https://github.com/JigSawFr/PlayGuard/releases/latest) e extraia-o na **raiz** do cartão SD. O app fica em `sd:/switch/playguard/`, e os arquivos dele em `sd:/config/playguard/`.
- **sphaira › GitHub** — O zip da versão já contém a entrada (`/config/sphaira/github/playguard.json`): depois de uma primeira instalação, atualize pelo *GitHub* no sphaira.

O **sysmodule de recuperação** opcional é instalado pelo PlayGuard (*Console › Módulos opcionais*, ou *Segurança › Sem acesso?*), ou manualmente pelo download próprio dele (`playguard-rescue.zip`): extraia-o também na raiz do cartão SD e depois reinicie o console. Ele fica em `atmosphere/contents/4200000000505247/` e só age quando um arquivo `RESCUE` pede — veja [Console bloqueado?](troubleshooting.md#console-bloqueado).

Depois, abra o PlayGuard **por cima de um jogo**, com o usuário de um responsável ([por quê](compatibility.md#launchers)).

## Atualizar

- **No PlayGuard:** *Preferências › Atualizar com* escolhe como — no próprio PlayGuard (ele baixa a versão, verifica, se substitui e reinicia), pelo sphaira, pela Homebrew App Store ou manualmente. Com *Procurar atualizações todos os dias* ativado, o PlayGuard abre mostrando a nova versão quando houver uma.
- **O sysmodule de recuperação** acompanha a versão do PlayGuard: depois de cada atualização, extraia de novo o `playguard-rescue.zip` dessa versão (ou *Atualizar* na entrada *O sysmodule de recuperação está desatualizado* do sino, ou em *Console › Módulos opcionais*) e depois reinicie o console.
- **O agente da conexão remota**, se estiver instalado, é oferecido na próxima abertura quando o PlayGuard traz um mais novo ([Home Assistant](../home-assistant.md#keep-the-link-up-when-playguard-is-closed), em inglês).
- As configurações, o histórico, os perfis e os backups ficam: eles estão em `sd:/config/playguard/`, não ao lado do app.

## Remover

Apague `sd:/switch/playguard/` (o app) e, para remover também os arquivos dele, `sd:/config/playguard/`; com os sysmodules opcionais, apague também `sd:/atmosphere/contents/4200000000505247/` (recuperação) e `sd:/atmosphere/contents/4200000000504741/` (o agente), ou remova-os antes em *Console › Módulos opcionais*. Os limites, o PIN e as restrições são do próprio console: eles **continuam no console**. Para removê-los também, use antes *Segurança › Excluir todo o controle dos pais*.

## Arquivos no cartão SD

Todos ficam em `sd:/config/playguard/` (o app em si fica em `sd:/switch/playguard/`: removê-lo ou reinstalá-lo mantém esses arquivos). As versões que os guardavam ao lado do app os moveram para lá na primeira vez que foram abertas.

- **`config.json`** — Preferências (idioma, tema, servidor NTP, *Proteger o PlayGuard com o PIN* …); cada chave em [config.md](../config.md) (em inglês). Se não puder ser lido, ele é guardado como `config.json.bad` (o anterior como `.bad.1`) e as configurações voltam aos valores padrão; o PlayGuard avisa uma vez ao abrir
- **`profiles/`** — Perfis de limite de tempo de jogo salvos
- **`exports/`** — Exportações da atividade
- **`nand/<system>/`** — O que pertence a um sistema, cada um com seu próprio controle parental: `sysmmc/` para o sysMMC, `emummc/` para o emuMMC. Dentro: `history.json` o histórico de alterações (as 500 mais recentes, incluindo as que o console recusou; se não puder ser lido, ele é guardado como `history.json.bad`, o anterior como `.bad.1`, em vez de ser sobrescrito, e o PlayGuard avisa uma vez); `state.json` o que *Bloquear todos os jogos*, o tempo extra, um rebloqueio pendente e a escolha de firmware precisam lembrar ([config.md](../config.md#state-kept-between-launches), em inglês); `forwarders.json` os forwarders criados pelo PlayGuard; `watch.json` o que o PlayGuard viu por último no console, para saber o que mudou fora dele ([config.md](../config.md#watchjson), em inglês; apagá-lo não causa problema); `backups/` os backups das configurações (nunca contêm o PIN); `cache/` a última atividade de jogo lida (todas as contas, e cada conta consultada), mostrada logo na próxima abertura enquanto o registro é lido de novo. Os arquivos de uma versão anterior a essas pastas vão para o primeiro sistema em que o PlayGuard for aberto
- **`cache/`** — Os ícones dos jogos (`icons/`, no máximo 16 MB, esvaziado quando o idioma do console muda), compartilhados por todos os sistemas, para que os nomes e os ícones não sejam lidos de novo de cada jogo a cada abertura; as notas de versão do Atmosphère, do hekate, do sys-patch e do firmware (`sys_notes.json`, para *Console*) e as do próprio PlayGuard (para *Sobre*); no modo de desenvolvedor, a lista de *Instalar outra compilação* (`dev_builds.json`)
- **`github_token`** — O login no GitHub de *Enviar um relatório…* e, no modo de desenvolvedor, de *Instalar outra compilação*; apagado ao sair da conta (*Sobre › Ajuda e diagnóstico › Conta do GitHub*, mostrado depois do login)
- **`gate.nro`**, **`gate-<hash>.nro`**, **`gate_*.txt`** — O portão dos forwarders protegidos (*Console › Forwarders*): o programa dele, os horários livres, as horas em que nunca abre e a pausa após recusas, as tentativas dele até o PlayGuard as ler, e quanto tempo dura a tela de abertura dele (`gate_wait.txt`, lido a cada abertura)
- **`previous_build.json`**, **`boot_check.json`** — Depois que uma compilação é instalada no lugar da atual: o nome da compilação substituída (guardada ao lado do `.nro` como `playguard.nro.previous`) e a verificação da próxima abertura (apagado assim que a primeira tela aparece)
- **`rescue_report.txt`** — Deixado pelo sysmodule de recuperação depois que ele agiu, até o PlayGuard mostrá-lo ao abrir (se não puder ser lido, ele é removido, e o PlayGuard avisa)
- **`rescue_nonce`** — Um desafio aleatório que o sysmodule de recuperação responde no relatório, para o PlayGuard distinguir um relatório escrito pelo sysmodule de um escrito à mão ([sysmodule/rescue/README.md](../../sysmodule/rescue/README.md), em inglês); um novo a cada abertura
- **`sync.conf`** — Só depois que o *Acesso remoto* é configurado: as configurações da conexão, incluindo a senha do broker (nunca em um relatório nem em um envio online)
- **`sync/`** — Só com o *Acesso remoto*: os perfis salvos, para o agente opcional (`profiles.txt`); o que ele lê e grava para um sistema fica no `nand/<system>/sync/` desse sistema (registros, nomes dos jogos, o que ele alterou, os eventos dele). Um sysMMC e um emuMMC compartilham `sync.conf`, mas são dois dispositivos no Home Assistant
- **`logs/`** — `reports/` os relatórios de diagnóstico salvos (nunca contêm o PIN nem o número de série); `crashes/` um arquivo por falha (o que fez o PlayGuard parar); `uploads/` um atalho de Internet (`.url`) por relatório enviado online; `play_timer/` os arquivos do temporizador das ferramentas de desenvolvedor. Quantos relatórios, arquivos de falha e links são mantidos: *Sobre › Ajuda e diagnóstico* (por quantidade, idade ou tamanho; o mais recente sempre fica)

Mais detalhes em [packaging/README.md](../../packaging/README.md) (em inglês).
