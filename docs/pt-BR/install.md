<!-- translated from docs/install.md, sha256 f12b746bb6b9 -->
# Instalação

Como instalar o PlayGuard e o que ele grava no cartão SD. Volte ao [README](../../README.md).

## Onde obter

Escolha uma opção:

- **Homebrew App Store** / **App Store do sphaira** (mesmo catálogo) — Procure *PlayGuard* depois que a ficha for aprovada. As novas versões são baixadas automaticamente.
- **sphaira › GitHub** — O zip da versão já contém a entrada (`/config/sphaira/github/playguard.json`): depois de uma primeira instalação, atualize pelo *GitHub* no sphaira.
- **Manual** — Baixe o `playguard.zip` da [versão mais recente](https://github.com/JigSawFr/PlayGuard/releases/latest) e extraia-o na **raiz** do cartão SD. O app fica em `sd:/switch/playguard/`, e os arquivos dele em `sd:/config/playguard/`.

O **sysmodule de recuperação** opcional (`playguard-rescue.zip`) é baixado à parte — veja [Console bloqueado?](troubleshooting.md#console-bloqueado).

## Arquivos no cartão SD

Todos ficam em `sd:/config/playguard/` (o app em si fica em `sd:/switch/playguard/`: removê-lo ou reinstalá-lo mantém esses arquivos). As versões que os guardavam ao lado do app os moveram para lá na primeira vez que foram abertas.

- **`config.json`** — Preferências (idioma, tema, servidor NTP, *Proteger o PlayGuard com o PIN* …); cada chave em [config.md](../config.md) (em inglês). Se não puder ser lido, ele é guardado como `config.json.bad` (o anterior como `.bad.1`) e as configurações voltam aos valores padrão; o PlayGuard avisa uma vez ao abrir
- **`profiles/`** — Perfis de limite de tempo de jogo salvos
- **`exports/`** — Exportações da atividade
- **`nand/<system>/`** — O que pertence a um sistema, cada um com seu próprio controle parental: `sysmmc/` para o sysMMC, `emummc/` para o emuMMC. Dentro: `history.json` o histórico de alterações (as 500 mais recentes, incluindo as que o console recusou; se não puder ser lido, ele é guardado como `history.json.bad`, o anterior como `.bad.1`, em vez de ser sobrescrito, e o PlayGuard avisa uma vez); `state.json` o que *Bloquear todos os jogos*, o tempo extra, um rebloqueio pendente e a escolha de firmware precisam lembrar ([config.md](../config.md#state-kept-between-launches), em inglês); `watch.json` o que o PlayGuard viu por último no console, para saber o que mudou fora dele ([config.md](../config.md#watchjson), em inglês; apagá-lo não causa problema); `backups/` os backups das configurações (nunca contêm o PIN); `cache/` a última atividade de jogo lida (todas as contas, e cada conta consultada), mostrada logo na próxima abertura enquanto o registro é lido de novo. Os arquivos de uma versão anterior a essas pastas vão para o primeiro sistema em que o PlayGuard for aberto
- **`cache/`** — Os ícones dos jogos (`icons/`, no máximo 16 MB, esvaziado quando o idioma do console muda), compartilhados por todos os sistemas, para que os nomes e os ícones não sejam lidos de novo de cada jogo a cada abertura; as notas de versão do Atmosphère, do hekate e do firmware (`sys_notes.json`, para *Console*) e as do próprio PlayGuard (para *Sobre*); no modo de desenvolvedor, a lista de *Instalar outra compilação* (`dev_builds.json`)
- **`github_token`** — O login no GitHub de *Enviar um relatório online* e, no modo de desenvolvedor, de *Instalar outra compilação*; apagado ao sair da conta (*Sobre › Ajuda e diagnóstico › Conta do GitHub*, mostrado depois do login)
- **`previous_build.json`**, **`boot_check.json`** — Depois que uma compilação é instalada no lugar da atual: o nome da compilação substituída (guardada ao lado do `.nro` como `playguard.nro.previous`) e a verificação da próxima abertura (apagado assim que a primeira tela aparece)
- **`rescue_report.txt`** — Deixado pelo sysmodule de recuperação depois que ele agiu, até o PlayGuard mostrá-lo ao abrir (se não puder ser lido, ele é removido, e o PlayGuard avisa)
- **`rescue_nonce`** — Um desafio aleatório que o sysmodule de recuperação responde no relatório, para o PlayGuard distinguir um relatório escrito pelo sysmodule de um escrito à mão ([sysmodule/README.md](../../sysmodule/README.md), em inglês); um novo a cada abertura
- **`logs/`** — `reports/` os relatórios de diagnóstico salvos (nunca contêm o PIN nem o número de série); `crashes/` um arquivo por falha (o que fez o PlayGuard parar); `uploads/` um atalho de Internet (`.url`) por relatório enviado online; `play_timer/` os arquivos do temporizador das ferramentas de desenvolvedor. Quantos relatórios, arquivos de falha e links são mantidos: *Sobre › Ajuda e diagnóstico* (por quantidade, idade ou tamanho; o mais recente sempre fica)

Mais detalhes em [packaging/README.md](../../packaging/README.md) (em inglês).
