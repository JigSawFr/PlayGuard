<!-- translated from docs/install.md, sha256 900d9a183a17 -->
# Instalação

Como instalar, atualizar e remover o PlayGuard, e o que ele escreve no cartão SD. Depois de instalado: [como começar](getting-started.md). Voltar ao [README](../../README.md).

## Onde o obter

À escolha:

- **Homebrew App Store** / **App Store do sphaira** (o mesmo catálogo) — Procure *PlayGuard* e instale-o. As novas versões são obtidas automaticamente, depois de uma revisão: muitas vezes alguns dias depois do GitHub. O PlayGuard sabe que foi instalado a partir dali e recomenda atualizar através da loja.
- **Manualmente** — Transfira o `playguard.zip` da [versão mais recente](https://github.com/JigSawFr/PlayGuard/releases/latest) e extraia-o para a **raiz** do cartão SD. A aplicação fica em `sd:/switch/playguard/` e os seus ficheiros em `sd:/config/playguard/`.
- **sphaira › GitHub** — O zip da versão já contém a entrada (`/config/sphaira/github/playguard.json`): depois de uma primeira instalação, atualize a partir de *GitHub* no sphaira.

O **sysmodule de recuperação** opcional instala-se a partir do PlayGuard (*Consola › Módulos opcionais*, ou *Segurança › Sem acesso?*), ou manualmente a partir da sua própria transferência (`playguard-rescue.zip`): extraia-o também para a raiz do cartão SD e depois reinicie a consola. Fica em `atmosphere/contents/4200000000505247/` e só atua quando um ficheiro `RESCUE` o pede — veja [Consola bloqueada?](troubleshooting.md#consola-bloqueada).

Depois, abra o PlayGuard **sobre um jogo**, com o utilizador de um adulto ([porquê](compatibility.md#launchers)).

## Atualizar

- **No PlayGuard:** *Preferências › Atualizar com* escolhe a forma — no próprio PlayGuard (transfere a versão, verifica-a, substitui-se a si próprio e reinicia), o sphaira, a Homebrew App Store ou manualmente. Com *Procurar atualizações todos os dias* ativo, o PlayGuard abre na nova versão quando há uma.
- **O sysmodule de recuperação** acompanha a versão do PlayGuard: depois de cada atualização, extraia de novo o `playguard-rescue.zip` dessa versão (ou *Atualizar* na entrada *O sysmodule de recuperação está desatualizado* do sino, ou em *Consola › Módulos opcionais*) e depois reinicie a consola.
- **O agente da ligação remota**, se estiver instalado, é proposto no arranque seguinte quando o PlayGuard traz um mais recente ([Home Assistant](../home-assistant.md#keep-the-link-up-when-playguard-is-closed), em inglês).
- As definições, o histórico, os perfis e as cópias de segurança ficam: estão em `sd:/config/playguard/`, não junto da aplicação.

## Remover

Elimine `sd:/switch/playguard/` (a aplicação) e, para remover também os seus ficheiros, `sd:/config/playguard/`; com os sysmodules opcionais, também `sd:/atmosphere/contents/4200000000505247/` (recuperação) e `sd:/atmosphere/contents/4200000000504741/` (o agente), ou remova-os primeiro em *Consola › Módulos opcionais*. Os limites, o PIN e as restrições são da própria consola: **ficam na consola**. Para os remover também, use primeiro *Segurança › Eliminar todo o controlo parental*.

## Ficheiros no cartão SD

Todos em `sd:/config/playguard/` (a aplicação em si fica em `sd:/switch/playguard/`: removê-la ou reinstalá-la mantém-nos). As versões que os guardavam junto da aplicação mudaram-nos para aqui no primeiro arranque.

- **`config.json`** — Preferências (idioma, tema, servidor NTP, *Proteger o PlayGuard com o PIN*…); cada chave em [config.md](../config.md) (em inglês). Se não puder ser lido, é guardado como `config.json.bad` (o anterior como `.bad.1`) e as definições recomeçam dos valores predefinidos; o PlayGuard avisa uma vez ao arrancar
- **`profiles/`** — Perfis de limites de tempo de jogo guardados
- **`exports/`** — Exportações da atividade
- **`nand/<sistema>/`** — O que pertence a um sistema, cada um com o seu próprio controlo parental: `sysmmc/` para o sysMMC, `emummc/` para o emuMMC. Lá dentro: `history.json` o histórico de alterações (as 500 mais recentes, incluindo as que a consola recusou; se não puder ser lido, é guardado como `history.json.bad`, o anterior como `.bad.1`, em vez de ser substituído, e o PlayGuard avisa uma vez); `state.json` o que *Bloquear todos os jogos*, o tempo extra, um novo bloqueio pendente e a escolha do firmware precisam de lembrar ([config.md](../config.md#state-kept-between-launches), em inglês); `forwarders.json` os forwarders criados pelo PlayGuard; `watch.json` o que o PlayGuard viu da consola da última vez, para dizer o que mudou fora dele ([config.md](../config.md#watchjson), em inglês; eliminá-lo não faz mal); `backups/` as cópias de segurança das definições (nunca contêm o PIN); `cache/` a última atividade de jogo lida (todas as contas, e cada conta consultada), mostrada logo no arranque seguinte enquanto o registo é lido de novo. Os ficheiros de uma versão anterior a estas pastas vão para o primeiro sistema em que o PlayGuard arranca
- **`cache/`** — Os ícones dos jogos (`icons/`, 16 MB no máximo, esvaziada quando o idioma da consola muda), partilhados por todos os sistemas, para que os nomes e ícones não sejam lidos de novo de cada jogo a cada arranque; as notas da versão do Atmosphère, do hekate, do sys-patch e do firmware (`sys_notes.json`, para *Consola*) e as do próprio PlayGuard (para *Sobre*); no modo de programador, a lista de *Instalar outra compilação* (`dev_builds.json`)
- **`github_token`** — A sessão GitHub de *Enviar um relatório…* e, no modo de programador, de *Instalar outra compilação*; eliminado ao terminar a sessão (*Sobre › Ajuda e diagnóstico › Conta GitHub*, mostrado depois de iniciar sessão)
- **`gate.nro`**, **`gate-<hash>.nro`**, **`gate_*.txt`** — A porta dos forwarders protegidos (*Consola › Forwarders*): o seu programa, as suas horas livres, as horas em que nunca abre e a pausa após recusas, as suas tentativas até o PlayGuard as ler, e quanto tempo dura o seu ecrã de abertura (`gate_wait.txt`, lido a cada arranque)
- **`previous_build.json`**, **`boot_check.json`** — Depois de uma compilação ser instalada no lugar da atual: o nome da compilação substituída (guardada junto do `.nro` como `playguard.nro.previous`) e a verificação do arranque seguinte (eliminado assim que o seu primeiro ecrã aparece)
- **`rescue_report.txt`** — Deixado pelo sysmodule de recuperação depois de atuar, até o PlayGuard o mostrar ao arrancar (se não o puder ler, é removido, e o PlayGuard avisa)
- **`rescue_nonce`** — Um desafio aleatório a que o sysmodule de recuperação responde no seu relatório, para que o PlayGuard distinga um relatório escrito por ele de um escrito à mão ([sysmodule/rescue/README.md](../../sysmodule/rescue/README.md), em inglês); um novo a cada arranque
- **`sync.conf`** — Só depois de o *Acesso remoto* ser configurado: as definições da ligação, incluindo a palavra-passe do broker (nunca num relatório nem num envio online)
- **`sync/`** — Só com o *Acesso remoto*: os perfis guardados, para o agente opcional (`profiles.txt`); o que ele lê e escreve para um sistema está no `nand/<sistema>/sync/` desse sistema (registos, nomes dos jogos, o que alterou, os seus eventos). Um sysMMC e um emuMMC partilham `sync.conf`, mas são dois dispositivos no Home Assistant
- **`logs/`** — `reports/` os relatórios de diagnóstico guardados (nunca contêm o PIN nem o número de série); `crashes/` um ficheiro por falha (o que fez o PlayGuard parar); `uploads/` um atalho de Internet (`.url`) por relatório enviado online; `play_timer/` os ficheiros do temporizador de jogo das ferramentas de programador. Quantos relatórios, ficheiros de falha e ligações são mantidos: *Sobre › Ajuda e diagnóstico* (por número, idade ou tamanho; o mais recente fica sempre)

Mais em [packaging/README.md](../../packaging/README.md) (em inglês).
