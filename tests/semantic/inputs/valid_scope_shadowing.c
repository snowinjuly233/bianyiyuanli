int value = 1;

int main(int value) {
    int result = value;
    {
        int value = 2;
        int inner = value;
        if (inner) {
            int result = inner;
            return result;
        }
    }
    return result;
}
