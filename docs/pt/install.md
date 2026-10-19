<!-- translated from docs/install.md, sha256 f12b746bb6b9 -->
# Instalação

Como instalar o PlayGuard e o que ele escreve no cartão SD. Voltar ao [README](../../README.md).

## Onde o obter

À escolha:

- **Homebrew App Store** / **App Store do sphaira** (o mesmo catálogo) — Procure *PlayGuard* depois de a ficha ser aprovada. As novas versões são obtidas automaticamente.
- **sphaira › GitHub** — O zip da versão já contém a entrada (`/config/sphaira/github/playguard.json`): depois de uma primeira instalação, atualize a partir de *GitHub* no sphaira.
- **Manualmente** — Transfira o `playguard.zip` da [versão mais recente](https://github.com/JigSawFr/PlayGuard/releases/latest) e extraia-o para a **raiz** do cartão SD. A aplicação fica em `sd:/switch/playguard/` e os seus ficheiros em `sd:/config/playguard/`.

O **sysmodule de recuperação** opcional (`playguard-rescue.zip`) transfere-se à parte — veja [Consola bloqueada?](troubleshooting.md#consola-bloqueada).

## Ficheiros no cartão SD

Todos em `sd:/config/playguard/` (a aplicação em si fica em `sd:/switch/playguard/`: removê-la ou reinstalá-la mantém-nos). As versões que os guardavam junto da aplicação mudaram-nos para aqui no primeiro arranque.

- **`config.json`** — Preferências (idioma, tema, servidor NTP, *Proteger o PlayGuard com o PIN*…); cada chave em [config.md](../config.md) (em inglês). Se não puder ser lido, é guardado como `config.json.bad` (o anterior como `.bad.1`) e as definições recomeçam dos valores predefinidos; o PlayGuard avisa uma vez ao arrancar
- **`profiles/`** — Perfis de limites de tempo de jogo guardados
- **`exports/`** — Exportações da atividade
- **`nand/<sistema>/`** — O que pertence a um sistema, cada um com o seu próprio controlo parental: `sysmmc/` para o sysMMC, `emummc/` para o emuMMC. Lá dentro: `history.json` o histórico de alterações (as 500 mais recentes, incluindo as que a consola recusou; se não puder ser lido, é guardado como `history.json.bad`, o anterior como `.bad.1`, em vez de ser substituído, e o PlayGuard avisa uma vez); `state.json` o que *Bloquear todos os jogos*, o tempo extra, um novo bloqueio pendente e a escolha do firmware precisam de lembrar ([config.md](../config.md#state-kept-between-launches), em inglês); `watch.json` o que o PlayGuard viu da consola da última vez, para dizer o que mudou fora dele ([config.md](../config.md#watchjson), em inglês; eliminá-lo não faz mal); `backups/` as cópias de segurança das definições (nunca contêm o PIN); `cache/` a última atividade de jogo lida (todas as contas, e cada conta consultada), mostrada logo no arranque seguinte enquanto o registo é lido de novo. Os ficheiros de uma versão anterior a estas pastas vão para o primeiro sistema em que o PlayGuard arranca
- **`cache/`** — Os ícones dos jogos (`icons/`, 16 MB no máximo, esvaziada quando o idioma da consola muda), partilhados por todos os sistemas, para que os nomes e ícones não sejam lidos de novo de cada jogo a cada arranque; as notas da versão do Atmosphère, do hekate e do firmware (`sys_notes.json`, para *Consola*) e as do próprio PlayGuard (para *Sobre*); no modo de programador, a lista de *Instalar outra compilação* (`dev_builds.json`)
- **`github_token`** — A sessão GitHub de *Enviar um relatório online* e, no modo de programador, de *Instalar outra compilação*; eliminado ao terminar a sessão (*Sobre › Ajuda e diagnóstico › Conta GitHub*, mostrado depois de iniciar sessão)
- **`previous_build.json`**, **`boot_check.json`** — Depois de uma compilação ser instalada no lugar da atual: o nome da compilação substituída (guardada junto do `.nro` como `playguard.nro.previous`) e a verificação do arranque seguinte (eliminado assim que o seu primeiro ecrã aparece)
- **`rescue_report.txt`** — Deixado pelo sysmodule de recuperação depois de atuar, até o PlayGuard o mostrar ao arrancar (se não o puder ler, é removido, e o PlayGuard avisa)
- **`rescue_nonce`** — Um desafio aleatório a que o sysmodule de recuperação responde no seu relatório, para que o PlayGuard distinga um relatório escrito por ele de um escrito à mão ([sysmodule/README.md](../../sysmodule/README.md), em inglês); um novo a cada arranque
- **`logs/`** — `reports/` os relatórios de diagnóstico guardados (nunca contêm o PIN nem o número de série); `crashes/` um ficheiro por falha (o que fez o PlayGuard parar); `uploads/` um atalho de Internet (`.url`) por relatório enviado online; `play_timer/` os ficheiros do temporizador de jogo das ferramentas de programador. Quantos relatórios, ficheiros de falha e ligações são mantidos: *Sobre › Ajuda e diagnóstico* (por número, idade ou tamanho; o mais recente fica sempre)

Mais em [packaging/README.md](../../packaging/README.md) (em inglês).
