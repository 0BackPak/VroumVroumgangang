const char PAGE_HTML[] PROGMEM = R"rawliteral(

<!DOCTYPE html>
<html>
<head>
    <title>Serveur ESP32</title>
    <meta http-equiv='refresh' content='60' name='viewport' content='width=device-width, initial-scale=1' charset='UTF-8'/>
    <style>
        body {
            font-family: Arial, sans-serif;
            text-align: center;
        }
        .button-split-top-bot {
            display: inline-block;
            position: relative;
            width: 150px;
            height: 150px;
        }
        .button {
            position: absolute;
            width: 150px;
            height: 75px;
            border-radius: 75px;
            background-color: black;
            color: white;
            font-size: 20px;
            border: none;
            cursor: pointer;
        }

        .forward {
            top: 0;
            border-bottom: 1px solid white;
            border-radius: 75px 75px 0 0;
        }
        .backward {
            bottom: 0;
            border-top: 1px solid white;
            border-radius: 0 0 75px 75px;
        }


        .left {
            left: 0;
            width: 75px;
            height: 150px;
            border-radius: 75px 0 0 75px;
            background-color: black;
            color: white;
            font-size: 20px;
            border: none;
            cursor: pointer;
        }
        .right {
            right: 0;
            width: 75px;
            height: 150px;
            border-radius: 0 75px 75px 0;
            background-color: black;
            color: white;
            font-size: 20px;
            border: none;
            cursor: pointer;
        }
        
        
        
        .container {
            display: flex;
            justify-content: center;
            align-items: center;
            gap: 100px;
        }
        
        .left-right-container, .top-bottom-container {
            display: flex;
            justify-content: center;
            margin: 10px;
        }
    </style>
    <script>
        function sendCommand(command) {
            var xhr = new XMLHttpRequest();
            xhr.open("GET", "/" + command, true);
            xhr.send();
        }
    </script>
</head>
<body lang='fr'>
    <h1>Serveur</h1>
    <p>Ce serveur est hébergé sur un ESP32</p>
    <i>Créé par Vroum Vroum gang gang</i>
    <br><br>
    
    <div class="container">
        
        <div class="left-right-container">
            <button class="left" onclick="sendCommand('left')">Gauche</button>
            <button class="right" onclick="sendCommand('right')">Droite</button>
        </div>

        <div class="top-bottom-container">
            <div class="button-split-top-bot">
                <button class="button forward" onclick="sendCommand('forward')">Avant</button>
                <button class="button backward" onclick="sendCommand('backward')">Arrière</button>
            </div>
        </div>

    </div>

</body>
</html>

)rawliteral";
