import random
import taxi
import web
import json
import ctypes

urls = (
    "/", "Home",
    "/pos", "Position",
    "/query", "Query",
    "/about", "About"
)

def check(obj):
    results = obj["result"]
    nCars = len(results)
    assert nCars <= 5

    for result in results:
        assert len(result) == 6
        for i in range(5):
            assert isinstance(result[i], int)

        carId = result[0]
        nPlaces = result[1]
        dist = result[2]
        detour0 = result[3]
        detour1 = result[4]
        routes = result[5]

        assert carId <= 100000
        assert dist <= 10000
        assert detour0 <= 10000
        assert detour1 <= 10000
        assert len(routes) == nPlaces - 1

        for i in range(len(routes) - 1):
            assert routes[i][-1][0] == routes[i + 1][0][0]

class Position:
    def POST(self):
        data = json.loads(bytes.decode(web.data()))
        lng = data["lng"]
        lat = data["lat"]
        
        retStr = taxi.position(lng, lat)
        res = bytes.decode(ctypes.cast(retStr, ctypes.c_char_p).value)
        taxi.release(retStr)

        return json.dumps(json.loads(res))

class Query:
    def POST(self):
        data = json.loads(bytes.decode(web.data()))
        originId = data["origin"]
        destId = data["dest"]

        retStr = taxi.query(originId, destId)
        res = bytes.decode(ctypes.cast(retStr, ctypes.c_char_p).value)
        taxi.release(retStr)

        obj = json.loads(res)
        check(obj)

        return json.dumps(obj)

class Home:
    def GET(self):
        raise web.seeother("/static/home.html")

class About:
    def GET(self):
        raise web.seeother("/static/about.html")

if __name__ == "__main__":
    # initialization
    taxi.init()

    # run application
    app = web.application(urls, globals(), True)
    app.run()
