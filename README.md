# RazorCap
Simple plugin that allows to perform GPU captures with SceRazorCapture.
Works on both retail and devkit.

In order to make it work on retail, you'll need to dump and decrypt
`librazorcapture_es4.suprx` from the DevKit firmware and place it in
`ur0:data/librazorcapture_es4.suprx`.

To install the plugin, grab `RazorCap.suprx` from [Releases][latest-release]
and modify your taiHEN `config.txt` like so (where ABCD00001 is your app's
TITLEID):

```
*ABCD00001
ur0:tai/RazorCap.suprx
```

Press START + SELECT + D-PAD LEFT to perform a GPU capture.

## Credits
Thanks to Bythos for reporting sceRazorCapture as working on retail.

[latest-release]: https://github.com/Rinnegatamante/RazorCap/releases/latest
