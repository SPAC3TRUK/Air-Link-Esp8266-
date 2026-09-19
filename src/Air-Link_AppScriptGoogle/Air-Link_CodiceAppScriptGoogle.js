function doPost(e) {
  // Decodifica i dati ricevuti
  var dati = JSON.parse(e.postData.contents);

  // Ottieni l'anno corrente come stringa
  var anno = new Date().getFullYear().toString();

  // Ottieni il foglio di lavoro attivo
  var ss = SpreadsheetApp.getActiveSpreadsheet();
  var foglio = ss.getSheetByName(anno);

  // Se il foglio dell'anno non esiste, lo crea e aggiunge intestazioni
  if (!foglio) {
    foglio = ss.insertSheet(anno);
    foglio.appendRow([
      "Data e Ora", "TempMin", "TempMax", "UmidMin", "UmidMax",
      "Co2Min", "Co2Max", "PressioneMin", "PressioneMax"
    ]);
  }

  // Formatta la data e ora in modo leggibile (es. 29/07/2025 00:00)
  var dataOra = Utilities.formatDate(new Date(), Session.getScriptTimeZone(), "dd/MM/yyyy HH:mm");

  // Aggiunge i dati al foglio
  foglio.appendRow([
    dataOra,
    dati.tempMin,
    dati.tempMax,
    dati.umidMin,
    dati.umidMax,
    dati.co2Min,
    dati.co2Max,
    dati.pressioneMin,
    dati.pressioneMax
  ]);

  // Risposta di conferma
  return ContentService.createTextOutput("✅ Dati salvati nel foglio " + anno);
}

