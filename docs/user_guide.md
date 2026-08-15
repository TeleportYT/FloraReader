# User & Operating Guide: FloraReader

Welcome to **FloraReader**, your cute flower-themed e-reader for the LilyGO T5S 2.7" E-Paper display!

---

## 1. Operating the Buttons

Your LilyGO T5S board has 3 main navigation buttons on the side:

```
 [ 37 ]  ---------------->  Previous Page / Up
 [ 38 ]  ---------------->  Menu / Select / Open
 [ 39 ]  ---------------->  Next Page / Down
```

* **In Main Menu**:
  * `Button 37`: Move selection cursor UP.
  * `Button 39`: Move selection cursor DOWN.
  * `Button 38`: Select menu option.
* **In Book Reader**:
  * `Button 37`: Flip to PREVIOUS page.
  * `Button 39`: Flip to NEXT page.
  * `Button 38`: Save position & return to Main Menu.
* **In Library List**:
  * `Button 37` & `Button 39`: Scroll through books on SD card.
  * `Button 38`: Open selected book.

---

## 2. Uploading Books Wirelessly from Device

You don't need to take out the MicroSD card to add new books!

1. On the FloraReader Main Menu, select **`📲 WiFi Upload Portal`**.
2. FloraReader will display a cute screen with WiFi hotspot details:
   - **Network SSID**: `FloraReader-WiFi`
   - **Password**: `flower123`
   - **Web Address**: `http://192.168.4.1`
3. On your Device:
   - Open **Settings** -> **Wi-Fi**.
   - Connect to **FloraReader-WiFi**.
   - Open **Safari** or **Chrome** and go to **`http://192.168.4.1`**.
4. You will see a cute floral Web Upload page!
   - Tap **"Tap to Select Book (.txt / .md)"**.
   - Pick any text book or Markdown file on your iPhone.
   - The book will upload directly onto the MicroSD card in seconds!
5. When finished uploading, press **`Button 38`** on the e-reader to return to the Main Menu.

---

## 3. Automatic Bookmarking

FloraReader automatically remembers your exact reading position (book title, current page, and progress percentage) in internal ESP32 storage!

* When you turn off the e-reader or press `Button 38` to exit, your position is saved.
* Next time you open the e-reader, selecting **`🌸 Read Current Book`** instantly resumes right where you left off.
