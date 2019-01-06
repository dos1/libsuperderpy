var Module;
if (!Module) Module = (typeof Module !== 'undefined' ? Module : null) || {};
// Disable image and audio decoding
Module.noImageDecoding = true;
Module.noAudioDecoding = true;

// autoplay workaround
document.addEventListener("click", function() {
  try {
    if (!SDL2 || !SDL2.audioContext || !SDL2.audioContext.resume) return;
      SDL2.audioContext.resume();
    } catch (err) {}
  }
);
