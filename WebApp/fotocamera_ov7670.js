var xres = 80;  // Larghezza foto 
var yres = 60;  // Altezza foto
var ctx;        // Serve per la "stampa" della foto
var imgData;    // Serve per la stampa della foto
var gcanvasid = "canvas";   // id del canvas che conterra' la foto
let scaleFactor = 1;    // Initial scale factor


function hexStringToUint8Array(hexString) {
    // Remove any non-hex characters (like spaces or newline characters)
    hexString = hexString.replace(/[^0-9a-fA-F]/g, '');
  
    // Split the string into pairs of two
    const pairs = hexString.match(/.{1,2}/g);
  
    // Convert each pair to a byte value
    const byteArray = pairs.map((pair) => parseInt(pair, 16));
  
    // Create a Uint8Array from the byte values
    return new Uint8Array(byteArray);
  }
  
  

// Funzione che server per inizializzare le variabili della foto
function init_canvas() {
    
    canvas = document.getElementById("canvas");
    canvas.width = xres;
    canvas.height = yres;

    ctx = canvas.getContext("2d");
    imgData = ctx.createImageData(canvas.width, canvas.height);
    for (var i = 0; i < imgData.data.length; i += 4) {
        imgData.data[i + 0] = 0xCC;
        imgData.data[i + 1] = 0xCC;
        imgData.data[i + 2] = 0xCC;
        imgData.data[i + 3] = 255;
    }
    ctx.putImageData(imgData, canvas.width, canvas.height);
    ctx.globalAlpha = 0.5;

}

// Funzione che "stampa" l'immagine nel canvas
function display(pixels) {
    //alert('display'); 
    ln = 0;
    var i = 0;
    for (y = 0; y < yres; y++) {
        for (x = 0; x < xres; x++) {
            i = (y * xres + x) << 1;
            pixel16 = (0xffff & pixels[i]) | ((0xffff & pixels[i + 1]) << 8);
            imgData.data[ln + 0] = ((((pixel16 >> 11) & 0x1F) * 527) + 23) >> 6;
            imgData.data[ln + 1] = ((((pixel16 >> 5) & 0x3F) * 259) + 33) >> 6;
            imgData.data[ln + 2] = (((pixel16 & 0x1F) * 527) + 23) >> 6;
            imgData.data[ln + 3] = (0xFFFFFFFF) & 255;
            ln += 4;
        }
    }
    ctx.putImageData(imgData, 0, 0);
}




