#include <iostream>
#include <vector>
#include <cstdlib>
#include <cstring>

class CustomString
{
public:
    CustomString() = default;

    ~CustomString()
    {
        if (m_data) delete[] m_data;
        m_data = nullptr;
        m_length = 0;
    }

    CustomString(CustomString&& other)
    {
        m_data = other.m_data;
        m_length = other.m_length;
        other.m_data = nullptr;
        other.m_length = 0;
    }

    CustomString(const CustomString& other)
    {
        raw_resize(other.len());
        memcpy(m_data, other.m_data, other.len());
    }

    CustomString(const char *str)
    {
        size_t str_size = 0;
        for (const char *p = str; *p; p++)
        {
            str_size++;
        }

        raw_resize(str_size);
        memcpy(m_data, str, str_size);
    }

    CustomString& operator=(const char *str)
    {
        this->~CustomString();
        new(this) CustomString(str);
        return *this;
    }

    size_t len() const { return m_length; }

    CustomString sub(size_t start, size_t count)
    {
        if (count <= 0 || start >= this->len())
        {
            return "";
        }

        size_t str_size = std::min(count, this->len() - start);
        CustomString ret;
        ret.raw_resize(str_size);
        memcpy(ret.m_data, m_data + start, str_size);
        return ret;
    }

    void append(CustomString str)
    {
        uint8_t *new_data = new uint8_t[len() + str.len() + 1];
        memcpy(new_data, m_data, len());
        memcpy(new_data + len(), str.m_data, str.len());
        new_data[len() + str.len()] = 0;
        if (m_data) delete[] m_data;
        m_data = new_data;
        m_length = len() + str.len();
    }

    bool operator==(const CustomString &other) const
    {
        if (len() != other.len())
        {
            return false;
        }

        for (int i = 0; i < len(); i++)
        {
            if (m_data[i] != other[i])
            {
                return false;
            }
        }

        return true;
    }

    uint8_t operator[](int index) const { return m_data[index]; }
    uint8_t &operator[](int index) { return m_data[index]; }

    int find(const CustomString pattern, int start_pos = 0)
    {
        if (pattern.len() == 0)
        {
            return start_pos;
        }

        std::vector<int> next(pattern.len(), 0); // KMP next
        int j = 0;
        for (int i = 1; i < pattern.len(); i++)
        {
            while (j > 0 && pattern[i] != pattern[j])
            {
                j = next[j - 1];
            }
            if (pattern[i] == pattern[j])
            {
                j++;
            }
            next[i] = j;
        }

        j = 0;
        for (int i = start_pos; i < len(); i++)
        {
            while (j > 0 && m_data[i] != pattern[j])
            {
                j = next[j - 1];
            }
            if (m_data[i] == pattern[j])
            {
                j++;
            }
            if (j == pattern.len())
            {
                return (i - pattern.len() + 1);
            }
        }

        return -1;
    }

    std::vector<CustomString> split(CustomString delimiter)
    {
        std::vector<CustomString> tokens;
        int start = 0, end = 0;

        if (delimiter.len() > 0)
        {
            while ((end = find(delimiter, start)) != -1)
            {
                tokens.push_back(sub(start, end - start));
                start = end + delimiter.len();
            }
        }

        tokens.push_back(sub(start, len() - start));

        return tokens;
    }

private:
    void raw_resize(size_t str_size)
    {
        if(str_size == m_length)
        {
            return;
        }

        if (m_data) delete[] m_data;
        m_data = new uint8_t[str_size + 1];
        m_data[str_size] = 0;
        m_length = str_size;
    }

private:
    uint8_t *m_data = nullptr;
    size_t m_length = 0;
};

int main()
{
    auto str1 = CustomString("test1");
    auto str2 = CustomString("test2, test3");
    str1 = "test3";
    int len = str1.len();
    CustomString sub1 = str1.sub(1, 2);
    str1.append("append");
    bool equal = str1 == str2;
    int index = str1.find("es");
    std::vector<CustomString> ret = str2.split(",");
    return 0;
}