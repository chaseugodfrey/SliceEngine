using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace SliceEngine
{
    /// <summary>
    /// Provides lightweight CSV loading and data lookup for designer-authored tables.
    /// </summary>
    public class SliceCSV
    {
        private List<string> _headers = new List<string>();
        private List<Dictionary<string, string>> _rows = new List<Dictionary<string, string>>();

        public int RowCount => _rows.Count;
        public IReadOnlyList<string> Headers => _headers.AsReadOnly();

        /// <summary>
        /// Loads a CSV file from a given path.
        /// Assumes the first line contains headers.
        /// </summary>
        public void Load(string filePath)
        {
            _headers.Clear();
            _rows.Clear();

            var lines = File.ReadAllLines(filePath);
            if (lines.Length == 0)
                return;

            _headers.AddRange(lines[0].Split(','));

            for (int i = 1; i < lines.Length; ++i)
            {
                string[] values = lines[i].Split(',');
                var row = new Dictionary<string, string>();

                for (int j = 0; j < _headers.Count && j < values.Length; ++j)
                    row[_headers[j]] = values[j].Trim();

                _rows.Add(row);
            }
        }

        /// <summary>
        /// Retrieves a value by row index and column name.
        /// </summary>
        public string GetValue(int rowIndex, string columnName)
        {
            if (rowIndex < 0 || rowIndex >= _rows.Count)
                throw new IndexOutOfRangeException();

            if (!_rows[rowIndex].TryGetValue(columnName, out var value))
                throw new ArgumentException($"Column '{columnName}' not found.");

            return value;
        }

        /// <summary>
        /// Attempts to convert a value to the specified type (e.g. int, float).
        /// </summary>
        public T GetValue<T>(int rowIndex, string columnName)
        {
            string value = GetValue(rowIndex, columnName);
            return (T)Convert.ChangeType(value, typeof(T));
        }

        /// <summary>
        /// Finds a row by a unique key value (e.g., unit name or ID).
        /// </summary>
        public Dictionary<string, string> FindRow(string keyColumn, string keyValue)
        {
            foreach (var row in _rows)
            {
                if (row.TryGetValue(keyColumn, out var val) && val == keyValue)
                    return row;
            }
            return null;
        }
    }
}
