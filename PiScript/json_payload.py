url = "https://es96app.herokuapp.com/test"

payload_template = {
    "scan_id": "",
    "time": "",
    "username": "ES96",
    "device": "prototype_1",

    "tags": [
        {
            # INSERT TAGS HERE
        }
    ],

    "data": [
        {
            "light_spectrum": [],
            "sound_attenuation": None,
            "impedance_sweep": None
        }
    ]
}

headers = {
    'Content-Type': 'application/json'
}

def populate_payload(scan_id="", time="", light_spectrum=[], sound_atten=None, impedance_sweep=None):
    global payload_template
    payload = payload_template
    payload["scan_id"] = scan_id
    payload["time"] = time
    payload["data"][0]["light_spectrum"] = light_spectrum
    payload["data"][0]["sound_attenuation"] = sound_atten
    payload["data"][0]["impedance_sweep"] = impedance_sweep
    return payload

