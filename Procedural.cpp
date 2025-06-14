#pragma comment(linker, "/STACK:2000000000")
#pragma comment(linker, "/HEAP:2000000000")

#include <iostream>
#include <SFML/Graphics.hpp>
#include <time.h>
#include <cstdlib>
#include <math.h>
#include <map>
#include <iomanip>
#include <sstream>
#include <thread>
#include <chrono>
#include <future>
using namespace std;
using namespace sf;

int screenSize = 512;

class RGB {
public:
    int R, G, B;
    RGB(int r = 0, int g = 0, int b = 0) : R(r), G(g), B(b) {}
};

class Map {
public:
    using Grid = vector<vector<float>>;
    int seed;
    int layers;
    float zoom;
    float multiplier;
    float amplifier;
    int rotation;
    int baseSize;
    int mapTileSize;
    float xOffset;
    float yOffset;
    float xOffsetLimit;
    float yOffsetLimit;
    Grid map;
    Map(
        int s = 1248,
        int w = 128,
        int h = 128,
        int mts = 2,
        int r = 0,
        int l = 5,
        float m = 2,
        float a = 2,
        float z = 2
    ) : seed(s),layers(l),zoom(z),multiplier(m),amplifier(a),rotation(r),baseSize(w),mapTileSize(mts)
    {
        srand(s);
        map = Grid(baseSize, vector<float>(baseSize, 0));
        xOffset = 0;
        yOffset = 0;
    }

    Grid getMap(int call) {
        Grid tempArr(baseSize, vector<float>(baseSize, 0));
        if (call > 0) {
            rotation++;
        }
        if (call < 0) {
            rotation--;
        }
        if (rotation < 0) {
            rotation = 3;
        }
        if (rotation > 3) {
            rotation = 0;
        }
        tempArr = map;
        for (int i = 0; i < rotation; i++) {
            tempArr = rotateArray90(tempArr);
        }
        //tempArr = threeDArray(tempArr);
        return tempArr;
    }

    Grid generateMap() {
        vector<Grid> arrays;
        arrays.clear();
        for (int i = 0; i < layers; i++) {
            arrays.push_back(initArray(i+1));
        };

        Grid array;
        array = sumArrays(arrays);
        array = fallOffArray(array);
        array = amplifyArray(array);
        array = normalizeArray(array);
        return array;
    }

    Grid normalizeArray(Grid arr) {
        float min = 0;
        float max = 0;
        
        Grid tempArr(baseSize, vector<float>(baseSize, 0));
        for (int y = 0; y < baseSize; y++) {
            for (int x = 0; x < baseSize; x++) {
                if (arr[x][y] < min) {
                    min = arr[x][y];
                }
            }
        }
        for (int y = 0; y < baseSize; y++) {
            for (int x = 0; x < baseSize; x++) {
                tempArr[x][y] = arr[x][y] - min;
            }
        }

        for (int y = 0; y < baseSize; y++) {
            for (int x = 0; x < baseSize; x++) {
                if (tempArr[x][y] > max) {
                    max = tempArr[x][y];
                }
            }
        }
        if (max >= 256) {
            for (int y = 0; y < baseSize; y++) {
                for (int x = 0; x < baseSize; x++) {
                    tempArr[x][y] = (tempArr[x][y] / max) * 255;
                }
            }
        }
        return tempArr;
    }
    
    Grid amplifyArray(Grid arr) {
        Grid tempArr(baseSize, vector<float>(baseSize, 0));

        for (int y = 0; y < baseSize; y++) {
            for (int x = 0; x < baseSize; x++) {
                tempArr[x][y] = pow(arr[x][y], amplifier);
            }
        }

        return tempArr;
    }
    
    Grid rotateArray45(Grid arr) {
        Grid tempArr(baseSize, vector<float>(baseSize, 0));

        for (int y = 0; y < baseSize; y++) {
            for (int x = 0; x < baseSize; x++) {
                if (
                    x + y > (((baseSize + baseSize) / 2 - 1) / 2) &&
                    x + y <= (((baseSize + baseSize) / 2 - 1) * 1.5) &&
                    x - y < (((baseSize + baseSize) / 2 - 1) / 2) &&
                    x - y >= -(((baseSize + baseSize) / 2 - 1) / 2)
                    ) {
                    tempArr[x][y] = arr[x - (((baseSize - 1) / 2) - y)][y + (((baseSize - 1) / 2) - x)];
                }
                else {
                    tempArr[x][y] = 20;
                }
            }
        }

        return tempArr;
    }

    Grid rotateArray90(Grid arr) {
        Grid tempArr(baseSize, vector<float>(baseSize, 0));

        for (int y = 0; y < baseSize; y++) {
            for (int x = 0; x < baseSize; x++) {
                tempArr[x][y] = arr[y][baseSize-x-1];
            }
        }

        return tempArr;
    }
    
    Grid threeDArray(Grid arr) {
        Grid tempArr(baseSize, vector<float>(baseSize, 0));

        for (int y = 0; y < baseSize; y++) {
            for (int x = 0; x < baseSize; x++) {
                float z;
                z = pow((float)arr[x][y] / 16, 1);
                if (y - z > 0 && y - z < baseSize && arr[x][y] >= 0) {
                    tempArr[x][y - z] = arr[x][y];
                }
                else if (arr[x][y] < 0) {
                    tempArr[x][y] = 20;
                }
            }
        }
        
        return tempArr;
    }

    Grid smoothArray(Grid arr) {
        Grid tempArr(baseSize, vector<float>(baseSize, 0));
        for (int y = 0; y < baseSize; y++) {
            for (int x = 0; x < baseSize; x++) {
                if (x > 0 && x < baseSize - 1 && y > 0 && y < baseSize - 1) {
                    float sum = 0;
                    for (int dy = -1; dy <= 1; dy++) {
                        for (int dx = -1; dx <= 1; dx++) {
                            sum += arr[x + dx][y + dy];
                        }
                    }
                    tempArr[x][y] = sum / 9;
                }
                else {
                    tempArr[x][y] = rand() % 2;
                }
            }
        }
        return tempArr;
    }

    Grid zoomArray(Grid arr) {
        Grid tempArr(baseSize, vector<float>(baseSize, 0));
        for (int y = 1; y < (baseSize) / (zoom)+1; y++) {
            for (int x = 1; x < (baseSize) / (zoom)+1; x++) {
                for (int dy = 0; dy < zoom; dy++) {
                    for (int dx = 0; dx < zoom; dx++) {
                        tempArr[((x - 1) * zoom) + dx][((y - 1) * zoom) + dy] = arr[x][y];
                    }
                }
            }
        }
        return tempArr;
    }

    Grid initArray(int loopSize) {
        Grid tempArr(baseSize, vector<float>(baseSize, 0));
        for (int y = 0; y < baseSize; y++) {
            for (int x = 0; x < baseSize; x++) {
                tempArr[x][y] = rand() % 256;
            }
        }
        for (int i = 0; i < loopSize; i++) {
            tempArr = smoothArray(tempArr);
            tempArr = zoomArray(tempArr);
            tempArr = smoothArray(tempArr);
        }
        return tempArr;
    }

    Grid sumArrays(vector<Grid> arrs) {
        Grid tempArr(baseSize, vector<float>(baseSize, 0));
        for (int y = 0; y < baseSize; y++) {
            for (int x = 0; x < baseSize; x++) {
                for (int i = 0; i < arrs.size(); i++) {
                    tempArr[x][y] += arrs[i][x][y];
                }
                tempArr[x][y] /= arrs.size();
            }
        }
        return tempArr;
    }

    Grid fallOffArray(Grid arr) {
        Grid tempArr(baseSize, vector<float>(baseSize, 0));
        for (int y = baseSize / -2; y < baseSize / 2; y++) {
            for (int x = baseSize / -2; x < baseSize / 2; x++) {
                tempArr[x + (baseSize / 2)][y + (baseSize / 2)] =
                    arr[x + (baseSize / 2)][y + (baseSize / 2)] *
                    fallOffMultiplier(x, y);
            }
        }
        return tempArr;
    }
    
    float fallOffMultiplier(int x, int y) {
        float xNormalized = (float)x / (float)baseSize;
        float yNormalized = (float)y / (float)baseSize;
        float finalizedMultiplier = (1 -
            (
                pow(abs(xNormalized), multiplier) +
                pow(abs(yNormalized), multiplier)
            )
        );
        return finalizedMultiplier;
    }
};

map<int, RGB> fillColorMap(map<int, RGB> tempArr) {
    int startKey = -1;
    int endKey = -1;
    for (int j = endKey + 1; j < 256; j = endKey + 1, startKey = -1, endKey = -1) {
        for (int i = j; i < 256; i++) {
            if (tempArr[i].R == 0 && tempArr[i].G == 0 && tempArr[i].B == 0) {
                if (startKey == -1) {
                    if (i != 0) {
                        startKey = i - 1;
                    }
                    else {
                        startKey = 0;
                    }
                }
            }
        }
        for (int i = startKey + 1; i < 256; i++) {
            if (!(tempArr[i].R == 0 && tempArr[i].G == 0 && tempArr[i].B == 0)) {
                if (endKey == -1) {
                    endKey = i;
                }
            }
        }
        for (int i = startKey; i <= endKey; i++) {
            tempArr[i] = RGB(
                tempArr[startKey].R + ((i - startKey) * (tempArr[endKey].R - tempArr[startKey].R) / (endKey - startKey)),
                tempArr[startKey].G + ((i - startKey) * (tempArr[endKey].G - tempArr[startKey].G) / (endKey - startKey)),
                tempArr[startKey].B + ((i - startKey) * (tempArr[endKey].B - tempArr[startKey].B) / (endKey - startKey))
            );
        }
    }
    return tempArr;
}

map<int, RGB> colorMap = {
    {0, RGB(0, 0, 0)}, //Black
    {80, RGB(0, 6, 73)}, //Deep Water
    {110, RGB(0, 44, 255)}, //Shallow Water
    {130, RGB(255, 236, 107)}, //Sand
    {140, RGB(137, 81, 20)}, //Dirt
    {150, RGB(12, 100, 50)}, //Grass
    {195, RGB(137, 81, 20)}, //Dirt
    {200, RGB(100, 100, 100)}, //Dark Grey
    {225, RGB(200, 200, 200)}, //Light Grey
    {255, RGB(255, 255, 255)} //White
};

void screenShot(RenderWindow& window) {
    Texture texture;
    texture.create(window.getSize().x, window.getSize().y);
    texture.update(window);

    Image screenshot = texture.copyToImage();
    cout << "Enter File Name: " << endl;
    string fileName;
    cin >> fileName;

    // Save file
    if (screenshot.saveToFile("Images\\" + fileName + ".png")) {
        cout << "Saved screenshot: " << fileName << "\n";
    }
    else {
        cout << "Failed to save screenshot.\n";
    }
}

void mapWindow(int seedText, int baseSize) {
    //Initialization
    RenderWindow window(VideoMode(screenSize, screenSize), "The Game!");

    Map map(seedText, baseSize);
    map.map = map.generateMap();
    Map::Grid cachedMap = map.getMap(0);

    Vector2i prevMousePos = Mouse::getPosition(window);

    float timer = 0, delay = 0.000001;
    Clock clock;
    Font font;
    font.loadFromFile("Minecraft.ttf");
    Text title(
        "Procedural Terrain Generation: ",
        font, 20
    ), seedTitle;
    seedTitle.setFont(font);
    seedTitle.setCharacterSize(30);

    title.move(5, 5);
    title.setFillColor(Color::White);
    seedTitle.move(5, screenSize - 35);
    seedTitle.setFillColor(Color::White);

    while (window.isOpen()) {
        float time = clock.getElapsedTime().asSeconds();
        clock.restart();
        timer += time;

        Event e;
        while (window.pollEvent(e))
        {
            //cout << map.xOffset << " " << map.yOffset << " " << map.mapTileSize << " "
            //     << map.xOffsetLimit << " " << map.yOffsetLimit << " " << map.baseSize << endl;
            //Events and Inputs
            Vector2i currentMousePos = Mouse::getPosition(window);

            float mouseX = currentMousePos.x - prevMousePos.x;
            float mouseY = currentMousePos.y - prevMousePos.y;
            prevMousePos = currentMousePos;

            if (e.type == Event::Closed) {
                window.close();
            }
            if (e.type == Event::MouseWheelScrolled) {
                if (e.mouseWheelScroll.wheel == Mouse::VerticalWheel) {
                    if (e.mouseWheelScroll.delta > 0 && map.mapTileSize < (float)screenSize) {
                        map.mapTileSize *= 2;
                        map.xOffsetLimit = -(int)(float)(map.baseSize - ((float)screenSize / map.mapTileSize));
                        map.yOffsetLimit = -(int)(float)(map.baseSize - ((float)screenSize / map.mapTileSize));
                        map.xOffset = (int)(float)(map.xOffset - ((float)screenSize / (2 * map.mapTileSize)));
                        map.yOffset = (int)(float)(map.yOffset - ((float)screenSize / (2 * map.mapTileSize)));
                    }
                    else if (e.mouseWheelScroll.delta < 0 && map.mapTileSize > ((float)screenSize / map.baseSize)) {
                        map.mapTileSize /= 2;
                        map.xOffset = (int)(float)(map.xOffset + ((float)screenSize / (4 * map.mapTileSize)));
                        map.yOffset = (int)(float)(map.yOffset + ((float)screenSize / (4 * map.mapTileSize)));
                        map.xOffsetLimit = -(int)(float)(map.baseSize - ((float)screenSize / map.mapTileSize));
                        map.yOffsetLimit = -(int)(float)(map.baseSize - ((float)screenSize / map.mapTileSize));
                    }
                }
            }
            if (
                Mouse::isButtonPressed(Mouse::Left) &&
                map.xOffset <= 0 &&
                map.yOffset <= 0 &&
                map.xOffset >= map.xOffsetLimit &&
                map.yOffset >= map.yOffsetLimit
                ) {
                map.xOffset += mouseX / map.mapTileSize;
                map.yOffset += mouseY / map.mapTileSize;

            }
            if (e.type == Event::KeyPressed && e.key.code == Keyboard::W && map.yOffset < 0) {
                map.yOffset += 1;
            }
            if (e.type == Event::KeyPressed && e.key.code == Keyboard::A && map.xOffset < 0) {
                map.xOffset += 1;
            }
            if (e.type == Event::KeyPressed && e.key.code == Keyboard::S && map.yOffset > -(map.baseSize - (screenSize / map.mapTileSize))) {
                map.yOffset -= 1;
            }
            if (e.type == Event::KeyPressed && e.key.code == Keyboard::D && map.xOffset > -(map.baseSize - (screenSize / map.mapTileSize))) {
                map.xOffset -= 1;
            }
            if (e.type == Event::KeyPressed && e.key.code == Keyboard::E) {
                cachedMap = map.getMap(1);
            }
            if (e.type == Event::KeyPressed && e.key.code == Keyboard::Q) {
                cachedMap = map.getMap(-1);
            }
            if (e.type == Event::KeyPressed && e.key.code == Keyboard::P) {
                screenShot(window);
            }
        }
        if (timer > delay)
        {
            timer = 0;
        }

        //Clamping
        if (map.xOffset > 0) { map.xOffset = 0; }
        if (map.yOffset > 0) { map.yOffset = 0; }
        if (map.xOffset < map.xOffsetLimit) { map.xOffset = map.xOffsetLimit; }
        if (map.yOffset < map.yOffsetLimit) { map.yOffset = map.yOffsetLimit; }
        if (map.mapTileSize < ((float)screenSize / map.baseSize)) {
            if (((float)screenSize / map.baseSize) < 1) {
                map.mapTileSize = 1;
            }
            else {
                map.mapTileSize = ((float)screenSize / map.baseSize);
            }
            map.xOffset = 0;
            map.yOffset = 0;
        }
        if (map.mapTileSize > (float)screenSize) {
            map.mapTileSize = (float)screenSize;
            map.xOffset = 0;
            map.yOffset = 0;
        }

        //Culling
        int tilesX = (float)screenSize / map.mapTileSize + 2;
        int tilesY = (float)screenSize / map.mapTileSize + 2;

        int startX = max(0, static_cast<int>(-floor(map.xOffset)) - 1);
        int startY = max(0, static_cast<int>(-floor(map.yOffset)) - 1);
        int endX = min(map.baseSize, startX + tilesX);
        int endY = min(map.baseSize, startY + tilesY);

        //Render
        window.clear(Color::White);
        seedTitle.setString("Seed: " + to_string(seedText));
        RectangleShape tile(Vector2f(map.mapTileSize, map.mapTileSize));
        for (int y = startY; y < endY; y++) {
            for (int x = startX; x < endX; x++)
            {
                RGB color = colorMap[cachedMap[x][y]];
                Color sfmlColor(color.R, color.G, color.B);
                tile.setPosition((x + map.xOffset) * map.mapTileSize, (y + map.yOffset) * map.mapTileSize);
                tile.setFillColor(sfmlColor);
                window.draw(tile);
            }
        }
        window.draw(title);
        window.draw(seedTitle);
        window.display();
    }
}

int main() {
    colorMap = fillColorMap(colorMap);
    while (true) {
        try {
                int seed;
                cout << "Enter Seed for Map Generation (or -1 to quit): ";
                cin >> seed;
                if (seed < 0) throw invalid_argument("Seed can't be negative");
                if (cin.fail()) {
                    cin.clear();
                    cin.ignore(10000, '\n');
                    throw invalid_argument("Input must be a number.");
                }
                mapWindow(seed, 128);
        }
        catch (const exception& e) {
            cerr << e.what() << endl;
        }
    }

    return 0;
}