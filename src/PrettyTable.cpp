#include "PrettyTable.h"

PrettyTable::PrettyTable() {
    m_BorderStyle.Corner = static_cast<Sign>(Sign::PT_PLUS);
    m_BorderStyle.H = static_cast<Sign>(Sign::PT_H);
    m_BorderStyle.V = static_cast<Sign>(Sign::PT_V);
    m_Rows = 0;
    m_Columns = 0;
    is_cancelFrame= false;
    is_DrawTable = false;
    m_Header._align = Align::Left;
}
PrettyTable::~PrettyTable() {
    _cleanup();
}
void PrettyTable::SetBorderStyle(Sign _Corner, Sign _Border_h, Sign _Border_v) {
    m_BorderStyle.Corner = _Corner;
    m_BorderStyle.H = _Border_h;
    m_BorderStyle.V = _Border_v;
}

PrettyTable::String PrettyTable::to_String(int _start, int _end) {
    if (!is_DrawTable)
        _draw_table(_start, _end);
    return m_innerOss.str();
}

void PrettyTable::AddHeader(const PrettyTable::StringArray& _header, Align _align) {
    if(_header.empty())
        throw Exception("Not have a _header_");
    if(_align!=Align::Left)
        m_Header._align = _align;
    m_Header._header = _header;
    m_Columns = m_Header._header.size();
    // resize the header columns
    m_multiColumns.resize(m_Columns);
    for (int i = 0; i < m_Columns; ++i)
        m_multiColumns[i].push_back(_header[i]);
}

void PrettyTable::AddRow(const PrettyTable::StringArray& _row) {
    if(_row.size()==0)
        return;
    m_multiRows.push_back(_row);
    m_Rows++;
    if ((size_t)m_Columns != _row.size())
        throw Exception("Header columns must be equal to Row columns!");
    _RowConvertColumn(_row);
}

void PrettyTable::AddRows(const PrettyTable::MultiStringArray& _rows) {
    for (auto p : _rows)
        AddRow(p);
}

void PrettyTable::AddColumn(const String& _header_title, const StringArray& _column) {
    if(m_Columns <= 0)return;
    m_Columns++;
    // resize the header columns
    m_multiColumns.push_back(_column);
    m_Header._header.push_back(_header_title);
    // convert column to a row
    for (int i = 0; i < m_Rows; ++i)
        m_multiRows[i].push_back(_column[i]);
}
/**
 * Start draw a table
 * Note that this function will draw all rows!
 * After you call this method, you call to_String(s,e) with two parameters, it is invalid
 */
void PrettyTable::DrawTable() {
    _draw_table(1, -1);
}

void PrettyTable::_draw_table(int s, int e) {
    _getMaximum_strLength();
    _draw_header();
    _draw_rows(s, e);
    is_DrawTable = true;
}
/**
 *  build header:
 *  +----------+------------------+----------+----------+---------------------+
 *  | xx       | xx              | xx       | xx       | xx                   |
 *  +----------+------------------+----------+----------+---------------------+
 */
void PrettyTable::_draw_header() {
    if (m_Columns <= 0)return;
    // draw top line
    // +----------+------------------+----------+----------+---------------------+
    if(!is_cancelFrame){
        _put_char(m_BorderStyle.Corner);
        for (int i = 0; i < m_Columns; ++i) {
            _put_chars(m_BorderStyle.H, m_ColumnsContent_max_len[i] + PADDING_LEFT_RIGHT);
            _put_char(m_BorderStyle.Corner);
        }
        _put_endline();
    }
    // draw header data
    if(!is_cancelFrame)
        _put_char(m_BorderStyle.V);

    for (int i = 0; i < m_Columns; ++i) {
        int _PADDING = 0;
        // Solve the Chinese characters problems
        // setw() = MaxSize + String.Size() - StringCapacity(String)
        // setw() = MaxSize + Length(Chinese hans)
        String str = m_Header._header[i];
        _PADDING = m_ColumnsContent_max_len[i] + _get_Chinese_len(str);

        m_innerOss << ' ';
        if(!is_cancelFrame){
            padding(_PADDING, m_Header._align);
        }
        if (m_Header._align == Align::Internal) {
            int s1 = (PADDING_LEFT_RIGHT / 2 + m_ColumnsContent_max_len[i]) / 2 + (StringCapacity(str)) / 2 + _get_Chinese_len(str);
            int s2 = m_ColumnsContent_max_len[i] - s1 + _get_Chinese_len(str);
            // set the center alignment
            m_innerOss << right << setw(s1)
                       << str
                       << left << setw(s2)
                       << "";
        }
        else {
            m_innerOss << str;
        }
        m_innerOss << ' ';
        if(!is_cancelFrame)
            _put_char(m_BorderStyle.V);
    }
    _put_endline();
    // draw bottom line
    // +----------+------------------+----------+----------+---------------------+
    if(!is_cancelFrame){
        _put_char(m_BorderStyle.Corner);
        for (int i = 0; i < m_Columns; ++i) {
            _put_chars(m_BorderStyle.H, m_ColumnsContent_max_len[i] + PADDING_LEFT_RIGHT);
            _put_char(m_BorderStyle.Corner);
        }
        _put_endline();
    }
}

void PrettyTable::_draw_rows(int s, int e) {
    if (m_Rows <= 0 || s < 0)return;
    if (s == 1 && e == -1) {
        s = 1;
        e =  m_Rows;
    }
    if (s > e)return;
    // deal with index overflow
    if (e >=  m_Rows) {
        e =  m_Rows;
    }
    for (int j = 0; j <  m_Rows; ++j) {
        if ((j + 1) < s || (j + 1) > e)continue;
        if(! is_cancelFrame)
            _put_char(m_BorderStyle.V);
        for (int i = 0; i <  m_Columns; ++i) {
            int _PADDING = 0;
            // Solve the Chinese characters problems
            // setw() = MaxSize + String.Size() - StringCapacity(String)
            // setw() = MaxSize + Length(Chinese hans)
            String str = m_multiRows[j][i];
            _PADDING =  m_ColumnsContent_max_len[i] + _get_Chinese_len(str);
            m_innerOss << ' ';
            if(! is_cancelFrame)
                padding(_PADDING, m_Header._align);
            // do with the internal aligning condition
            if (m_Header._align == Align::Internal) {
                int s1 = (PADDING_LEFT_RIGHT / 2 +  m_ColumnsContent_max_len[i]) / 2 + (StringCapacity(str)) / 2 + _get_Chinese_len(str);
                int s2 =  m_ColumnsContent_max_len[i] - s1 + _get_Chinese_len(str);
                // set the center alignment
                m_innerOss << right << setw(s1)
                           << str
                           << left << setw(s2)
                           << "";
            }else {
                m_innerOss << str;
            }
            m_innerOss << ' ';
            if(! is_cancelFrame)
                _put_char(m_BorderStyle.V);
        }
        _put_endline();
    }
    // draw bottom line
    // +----------+------------------+----------+----------+---------------------+
    if(! is_cancelFrame){
        _put_char(m_BorderStyle.Corner);
        for (int i = 0; i <  m_Columns; ++i) {
            _put_chars(m_BorderStyle.H,  m_ColumnsContent_max_len[i] + PADDING_LEFT_RIGHT);
            _put_char(m_BorderStyle.Corner);
        }
        _put_endline();
    }
}

const vector<int>& PrettyTable::_getMaximum_strLength() {
    for (const StringArray& p : m_multiColumns) {
        int m = StringCapacity(p[0]);
        int size= p.size();
        for (int i = 0; i < size; ++i) {
            if (m <= StringCapacity(p[i])) {
                // real size
                m = StringCapacity(p[i]);
            }
        }
        m_ColumnsContent_max_len.push_back(m);
    }
    return m_ColumnsContent_max_len;
}

void PrettyTable::_RowConvertColumn(const StringArray& _row) {
    for (int i = 0; i <  m_Columns; ++i) {
         m_multiColumns[i].push_back(_row[i]);
    }
}
void PrettyTable::_RowsConvertColumns() {
    for (auto x :  m_multiRows) {
        _RowConvertColumn(x);
    }
}
wstring PrettyTable::StringToWString(const string& str) {
    size_t nLen = str.length() * 2;
    setlocale(LC_CTYPE, "");
    wchar_t* wcs = new wchar_t[nLen];
    wmemset(wcs, 0, nLen);
    mbstowcs(wcs, str.c_str(), nLen);
    wstring w = wcs;
    delete[]wcs;
    return w;
}

int PrettyTable::StringCapacity(const string& raw_str) {
    return raw_str.size() - _get_Chinese_len(raw_str);
}

int PrettyTable::_get_Chinese_len(const string& str) {
    wstring ss = StringToWString(str);
    int sum = 0;
    for (size_t i = 0; i < ss.length(); ++i) {
        wchar_t wcs = ss.c_str()[i];
        if ((wcs >= (wchar_t)0x4E00u && wcs <= (wchar_t)0x9FFFu)
            || wcs == PC1 || wcs == PC2 || wcs == PC3 || wcs == PC4 || wcs == PC5 || wcs == PC6 || wcs == PC7 || wcs == PC8 || wcs == PC9
            || wcs == PC10 || wcs == PC11 || wcs == PC12 || wcs == PC13 || wcs == PC14 || wcs == PC15 || wcs == PC16 || wcs == PC17
            || wcs == PC18 || wcs == PC19 || wcs == PC20 || wcs == PC21 || wcs == PC22 || wcs == PC23 || wcs == PC24 || wcs == PC25)
        {
            sum++;
        }
    }
    return sum;
}

void PrettyTable::SetAlign(Align align) {
    m_Header._align = align;
}

void PrettyTable::OutputFile(const string& _file_path) {
    m_FileOut.open(_file_path);
    if (!m_FileOut.is_open())return;
    if (! is_DrawTable) DrawTable();
    m_FileOut << m_innerOss.str();
    m_FileOut.close();
}

void PrettyTable::_cleanup(){
    is_cancelFrame=false;
    m_Rows = 0;
    m_Columns = 0;
    m_Header._header.clear();
    m_multiColumns.clear();
    m_multiRows.clear();
    m_ColumnsContent_max_len.clear();
    m_innerOss.flush();
    m_innerOss.clear();
    m_innerOss.ignore();
    m_FileOut.clear();
}
