import java.io.IOException;

import yodautils.strings.StringUtils;

public class Parser {

    // Settings
    public static final String className = "Direct3DDevice9Wrapper";
    public static final String ifName = "Direct3DDevice9";
    public static final String fileName = "d3d9h_device.txt";

    // Internals
    public static final StringBuilder sb = new StringBuilder();

    public static class Param {

        final String preName;
        final String name;

        public Param(final String preName, final String name) {
            super();
            this.preName = preName;
            this.name = name;
        }

    }

    public static void main(final String[] args) throws IOException {
        for (String row : StringUtils.textFile2String(fileName).split("\n")) {
            row = row.trim();
            if (row.indexOf("STDMETHOD") == 0) {
                if (row.charAt(9) == '_') {
                    parseStdUnderscoreFcn(row);
                } else {
                    parseStdFcn(row);
                }
            }
        }
        System.out.println(sb.toString());
    }

    private static void produceSourceCode(final String outType, final String fcnName, String paramsString) {
        if (paramsString.indexOf("THIS") == 0) {
            int endThis = paramsString.indexOf(' ');
            paramsString = endThis > 0 ? paramsString.substring(endThis + 1, paramsString.length()) : "";
        }
        final String[] paramStrings = paramsString.split(",");
        sb.append(outType).append(' ').append(className).append("::").append(fcnName).append('(').append(paramsString)
                .append(") {\n");
        sb.append('\t');
        if (!outType.contains("void")) {
            sb.append("return ");
        }
        sb.append(ifName).append("->").append(fcnName).append('(');
        for (int i = 0; i < paramStrings.length; i++) {
            final String s = paramStrings[i];
            final String[] words = s.split(" ");
            String paramName = words[words.length - 1];
            if (paramName.length() > 0) {
                if (paramName.charAt(0) == '*') {
                    paramName = paramName.substring(1, paramName.length());
                }
                if (paramName.charAt(paramName.length() - 1) == '*') {
                    paramName = "param" + i;
                }
                sb.append(paramName);
                if (i + 1 < paramStrings.length) {
                    sb.append(", ");
                }
            }
        }
        sb.append(");\n");
        sb.append("}\n\n");
    }

    private static void parseStdUnderscoreFcn(final String row) {
        int fcnNameStartIndex = row.indexOf(',') + 1;
        if (row.charAt(fcnNameStartIndex) == ' ') {
            fcnNameStartIndex++;
        }
        final int fcnNameEndParanIndex = row.indexOf(')');
        final String fcnName = row.substring(fcnNameStartIndex, fcnNameEndParanIndex);
        final String paramsString = row.substring(fcnNameEndParanIndex + 2, row.lastIndexOf(')'));
        final String outType = row.substring(row.indexOf('(') + 1, row.indexOf(','));
        produceSourceCode(outType, fcnName, paramsString);
    }

    private static void parseStdFcn(final String row) {
        final int fcnNameStartIndex = 10;
        final int fcnNameEndParanIndex = row.indexOf(')');
        final String fcnName = row.substring(fcnNameStartIndex, fcnNameEndParanIndex);
        final String paramsString = row.substring(fcnNameEndParanIndex + 2, row.lastIndexOf(')'));
        final String outType = "HRESULT";
        produceSourceCode(outType, fcnName, paramsString);
    }

}
