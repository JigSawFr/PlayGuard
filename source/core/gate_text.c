// gate_text — the PIN gate's lines in the console's language (see gate_text.h).
// Its own few lines, apart from PlayGuard's catalogs (resources/i18n), which
// the gate does not carry; the tagline is the catalogs' app/tagline.
// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#include "gate_text.h"

typedef struct {
    const char *lines[GateText_Count];
} Lines;

static const Lines DE = { {
    "Gib die Geheimzahl der Jugendschutzeinstellungen ein, um es zu starten.",
    "Die Geheimzahl-Eingabe der Konsole öffnet sich.",
    "Geheimzahl eingegeben: wird gestartet…",
    "Freie Zeiten: gerade keine Geheimzahl nötig.",
    "Geheimzahl nicht eingegeben: zurück zum HOME-Menü.",
    "Bitte einen Elternteil, die Geheimzahl einzugeben.",
    "Die Geheimzahl-Eingabe konnte nicht geöffnet werden: zurück zum HOME-Menü.",
    "Altersbeschränkungen direkt auf der Konsole",
} };

static const Lines EN_US = { {
    "Enter the parental-control PIN to start it.",
    "The console's PIN screen opens now.",
    "PIN entered: starting…",
    "Free hours: no PIN needed right now.",
    "PIN not entered: back to the HOME menu.",
    "Ask a parent to enter the PIN.",
    "The PIN screen could not open: back to the HOME menu.",
    "Parental controls, right on the console",
} };

static const Lines ES = { {
    "Introduce el PIN del control parental para iniciarlo.",
    "Se abre la pantalla del PIN de la consola.",
    "PIN introducido: iniciando…",
    "Horario libre: ahora no hace falta el PIN.",
    "PIN no introducido: vuelta al menú HOME.",
    "Pide a un adulto que introduzca el PIN.",
    "No se pudo abrir la pantalla del PIN: vuelta al menú HOME.",
    "El control parental, directamente en la consola",
} };

static const Lines ES_419 = { {
    "Ingresa el PIN del control parental para iniciarlo.",
    "Se abre la pantalla del PIN de la consola.",
    "PIN ingresado: iniciando…",
    "Horario libre: ahora no hace falta el PIN.",
    "PIN no ingresado: regreso al menú HOME.",
    "Pide a un adulto que ingrese el PIN.",
    "No se pudo abrir la pantalla del PIN: regreso al menú HOME.",
    "El control parental, directamente en la consola",
} };

static const Lines FR = { {
    "Saisissez le code PIN parental pour le lancer.",
    "L'écran du code PIN de la console s'ouvre.",
    "Code PIN saisi : lancement…",
    "Horaires libres : pas besoin du code PIN maintenant.",
    "Code PIN non saisi : retour au menu HOME.",
    "Demandez à un parent de saisir le code PIN.",
    "L'écran du code PIN n'a pas pu s'ouvrir : retour au menu HOME.",
    "Le contrôle parental, directement sur la console",
} };

static const Lines FR_CA = { {
    "Entrez le NIP parental pour le lancer.",
    "L'écran du NIP de la console s'ouvre.",
    "NIP entré : lancement…",
    "Heures libres : pas besoin du NIP maintenant.",
    "NIP non entré : retour au menu HOME.",
    "Demandez à un parent d'entrer le NIP.",
    "L'écran du NIP n'a pas pu s'ouvrir : retour au menu HOME.",
    "Le contrôle parental, directement sur la console",
} };

static const Lines IT = { {
    "Inserisci il PIN del filtro famiglia per avviarlo.",
    "Si apre la schermata del PIN della console.",
    "PIN inserito: avvio…",
    "Orari liberi: ora non serve il PIN.",
    "PIN non inserito: ritorno al menu HOME.",
    "Chiedi a un genitore di inserire il PIN.",
    "Impossibile aprire la schermata del PIN: ritorno al menu HOME.",
    "Il filtro famiglia, direttamente sulla console",
} };

static const Lines JA = { {
    "起動するには、みまもり設定の暗証番号を入力してください。",
    "本体の暗証番号画面が開きます。",
    "暗証番号を確認しました：起動します…",
    "自由な時間帯です：今は暗証番号は不要です。",
    "暗証番号が入力されませんでした：HOMEメニューに戻ります。",
    "保護者に暗証番号を入力してもらってください。",
    "暗証番号画面を開けませんでした：HOMEメニューに戻ります。",
    "本体だけで使える、みまもり設定",
} };

static const Lines KO = { {
    "시작하려면 자녀 보호 PIN을 입력하세요.",
    "본체의 PIN 화면이 열립니다.",
    "PIN 확인: 시작합니다…",
    "자유 시간: 지금은 PIN이 필요 없습니다.",
    "PIN을 입력하지 않음: HOME 메뉴로 돌아갑니다.",
    "보호자에게 PIN을 입력해 달라고 하세요.",
    "PIN 화면을 열 수 없었습니다: HOME 메뉴로 돌아갑니다.",
    "본체에서 바로 쓰는 자녀 보호 기능",
} };

static const Lines NL = { {
    "Voer de pincode van het ouderlijk toezicht in om het te starten.",
    "Het pincodescherm van de console gaat open.",
    "Pincode ingevoerd: wordt gestart…",
    "Vrije uren: nu geen pincode nodig.",
    "Pincode niet ingevoerd: terug naar het HOME-menu.",
    "Vraag een ouder om de pincode in te voeren.",
    "Het pincodescherm kon niet openen: terug naar het HOME-menu.",
    "Ouderlijk toezicht, rechtstreeks op de console",
} };

static const Lines PT = { {
    "Introduza o PIN do controlo parental para o iniciar.",
    "O ecrã do PIN da consola abre-se agora.",
    "PIN introduzido: a iniciar…",
    "Horas livres: agora não é preciso o PIN.",
    "PIN não introduzido: regresso ao menu HOME.",
    "Peça a um adulto para introduzir o PIN.",
    "Não foi possível abrir o ecrã do PIN: regresso ao menu HOME.",
    "O controlo parental, diretamente na consola",
} };

static const Lines PT_BR = { {
    "Digite o PIN do controle dos pais para iniciar.",
    "A tela do PIN do console abre agora.",
    "PIN digitado: iniciando…",
    "Horário livre: agora não precisa do PIN.",
    "PIN não digitado: de volta ao menu HOME.",
    "Peça a um responsável para digitar o PIN.",
    "Não foi possível abrir a tela do PIN: de volta ao menu HOME.",
    "Controle dos pais direto no console",
} };

static const Lines RU = { {
    "Введите PIN-код родительского контроля, чтобы запустить.",
    "Открывается экран PIN-кода консоли.",
    "PIN-код введён: запуск…",
    "Свободные часы: сейчас PIN-код не нужен.",
    "PIN-код не введён: возврат в меню HOME.",
    "Попросите взрослого ввести PIN-код.",
    "Не удалось открыть экран PIN-кода: возврат в меню HOME.",
    "Родительский контроль прямо на консоли",
} };

static const Lines ZH_HANS = { {
    "请输入家长控制PIN以启动。",
    "主机的PIN界面即将打开。",
    "PIN已输入：正在启动…",
    "自由时段：现在无需PIN。",
    "未输入PIN：返回HOME菜单。",
    "请家长输入PIN。",
    "无法打开PIN界面：返回HOME菜单。",
    "直接在主机上使用的家长控制",
} };

static const Lines ZH_HANT = { {
    "請輸入家長監護PIN碼以啟動。",
    "主機的PIN碼畫面即將開啟。",
    "PIN碼已輸入：正在啟動…",
    "自由時段：現在不需PIN碼。",
    "未輸入PIN碼：返回HOME選單。",
    "請家長輸入PIN碼。",
    "無法開啟PIN碼畫面：返回HOME選單。",
    "直接在主機上使用的家長監護",
} };

// libnx's SetLanguage order (ja, en-US, fr, de, it, es, zh-CN, ko, nl, pt, ru,
// zh-TW, en-GB, fr-CA, es-419, zh-Hans, zh-Hant, pt-BR).
static const Lines *const BY_LANGUAGE[] = {
    &JA, &EN_US, &FR, &DE, &IT, &ES, &ZH_HANS, &KO, &NL, &PT, &RU, &ZH_HANT, &EN_US, &FR_CA, &ES_419, &ZH_HANS, &ZH_HANT, &PT_BR,
};

const char *gate_text(int language, GateText which)
{
    if ((int)which < 0 || which >= GateText_Count) return "";
    const Lines *l = language >= 0 && language < (int)(sizeof(BY_LANGUAGE) / sizeof(BY_LANGUAGE[0])) ? BY_LANGUAGE[language] : &EN_US;
    return l->lines[which];
}

GateFont gate_text_font(int language)
{
    switch (language) {
        case 6: case 15: return GateFont_ChineseSimplified;
        case 11: case 16: return GateFont_ChineseTraditional;
        case 7: return GateFont_Korean;
        default: return GateFont_Standard;
    }
}
