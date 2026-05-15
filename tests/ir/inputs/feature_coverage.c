int seed = 1;

int twice(int x) {
    return x + x;
}

int main() {
    int x = seed;
    int y = -x;
    y = twice(y + 3);
    if (!y) {
        y = y + 1;
    } else {
        y = y * 2;
    }
    while (y > 0) {
        y = y - 1;
    }
    return y;
}
