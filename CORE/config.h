const char MAIN_page[] PROGMEM =R"=====(
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>WifiGymManager</title>
</head>
<body style="width:100%;height:100vh;background-color: #344459; display:flex;justify-content: center;flex-direction: column;align-items: center;font-family: 'Courier New', Courier, monospace;">
    <style>
        *{
            box-sizing: border-box;
            margin: 0;
        }

        .form{
            width: 50%;
            display: flex;
            justify-content: center;
            align-items: center;
            flex-direction: column;
        }
        .form>div{
            width: 100%;
            display: grid;
            grid-template-columns: 30% 80%;
            justify-content: center;
            align-items: center;
            margin-bottom: 10px;
        }
        .form>div>label{
            color: white;
            text-align: center;
        }
        .form>div>input{
            width: 100%;
            height: 40px;
            border: none;
            padding-left: 10px;
            background-color: #aeb5be;
            border-radius: 5px;
        }
        .form>div>input:focus,.form>div>input:hover{
            outline: none;
            background-color: white
        }

        .button{
            width: 55%;
            height: 40px;
            display: block;
            border: none;
            background-color:rgb(50, 107, 187);
            color: white;
            cursor: pointer;
            border-radius: 5px;
            font-size: 14px;
        }
        .button:focus,.button:hover{
            background-color: rgb(96, 164, 241);
        }

        @media screen and (min-width:300px) and (max-width:500px){
            .form{
                width: 90%;
            }
            .form>div{
                grid-template-columns: 1fr;
            }
            .form>div>label{
                text-align: left;
            }
            .form>div>input{
                background-color: #c5c6c7;
            }

            .button{
                width: 100%;
                color: white;
            }
        }

    </style>

    <h1 style="color: white;text-align: center;margin: 50px 0;">Configura tu dispositivo</h1>
    
    <form class="form" action="/saveEEPROM" method="GET">
        <div>
            <label>SSID:</label>
            <input name="ssid" required type="text" placeholder="Nombre de la red WiFi">
        </div>
        <div>
            <label>Contraseña:</label>
            <input name="pass" required type="text" placeholder="Ingresa la contraseña del WiFi">
        </div>
        <div>
            <label>IP del servidor:</label>
            <input name="ip" required type="text" placeholder="Ingresa la IP del computador">
        </div>
        <button class="button" type="submit">Guardar configuración</button>
    </form>
</body>
</html>
)=====";