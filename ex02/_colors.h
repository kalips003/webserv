/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   _colors.h                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: kalipso <kalipso@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2022/09/14 14:19:41 by kmendes-          #+#    #+#             */
/*   Updated: 2024/09/23 15:06:53 by kalipso          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef _COLORS_H
# define _COLORS_H

///////////////////////////////////////////////////////////////////////////////]
# define ERR "\e[0;31mError - \e[0m"
# define ERRM "\e[0;48;1mError ( MALLOC )\n\e[0m"

# define ERR0 "\e[38;5;15mError (0) \e[0m"
# define ERR1 "\e[38;5;123mError (1) \e[0m"
# define ERR2 "\e[38;5;27mError (2) \e[0m"
# define ERR3 "\e[38;5;214mError (3) \e[0m"
# define ERR4 "\e[38;5;99mError (4) \e[0m"
# define ERR5 "\e[38;5;55mError (5) \e[0m"
# define ERR6 "\e[38;5;202mError (6) \e[0m"
# define ERR7 "\e[38;5;51mError (7) \e[0m"
# define ERR8 "\e[38;5;78mError (8) \e[0m"
# define ERR9 "\e[38;5;189mError (9) \e[0m"

# define CLEAR "\e[2J\e[H"
# define CLS "\e[2J\e[H"
# define REVERSE "\e[7m"
# define BLINK "\e[5m"
# define R_BLINK "\e[25m"

# define RESET "\e[0m"
# define END "\e[0m\n"

# define BLACK "\e[0;30m"
# define RED "\e[0;31m"
# define GREEN "\e[0;32m"
# define YELLOW "\e[0;33m"
# define BLUE "\e[0;34m"
# define PURPLE "\e[0;35m"
# define CYAN "\e[0;36m"
# define WHITE "\e[0;37m"

# define E0 "\e[48;30mError (0)- \e[0m"

# define C_000 "\e[38;5;16m"
# define C_001 "\e[38;5;17m"
# define C_002 "\e[38;5;18m"
# define C_003 "\e[38;5;19m"
# define C_004 "\e[38;5;20m"
# define C_005 "\e[38;5;21m"
# define C_010 "\e[38;5;22m"
# define C_011 "\e[38;5;23m"
# define C_012 "\e[38;5;24m"
# define C_013 "\e[38;5;25m"
# define C_014 "\e[38;5;26m"
# define C_015 "\e[38;5;27m"
# define C_020 "\e[38;5;28m"
# define C_021 "\e[38;5;29m"
# define C_022 "\e[38;5;30m"
# define C_023 "\e[38;5;31m"
# define C_024 "\e[38;5;32m"
# define C_025 "\e[38;5;33m"
# define C_030 "\e[38;5;34m"
# define C_031 "\e[38;5;35m"
# define C_032 "\e[38;5;36m"
# define C_033 "\e[38;5;37m"
# define C_034 "\e[38;5;38m"
# define C_035 "\e[38;5;39m"
# define C_040 "\e[38;5;40m"
# define C_041 "\e[38;5;41m"
# define C_042 "\e[38;5;42m"
# define C_043 "\e[38;5;43m"
# define C_044 "\e[38;5;44m"
# define C_045 "\e[38;5;45m"
# define C_050 "\e[38;5;46m"
# define C_051 "\e[38;5;47m"
# define C_052 "\e[38;5;48m"
# define C_053 "\e[38;5;49m"
# define C_054 "\e[38;5;50m"
# define C_055 "\e[38;5;51m"
# define C_100 "\e[38;5;52m"
# define C_101 "\e[38;5;53m"
# define C_102 "\e[38;5;54m"
# define C_103 "\e[38;5;55m"
# define C_104 "\e[38;5;56m"
# define C_105 "\e[38;5;57m"
# define C_110 "\e[38;5;58m"
# define C_111 "\e[38;5;59m"
# define C_112 "\e[38;5;60m"
# define C_113 "\e[38;5;61m"
# define C_114 "\e[38;5;62m"
# define C_115 "\e[38;5;63m"
# define C_120 "\e[38;5;64m"
# define C_121 "\e[38;5;65m"
# define C_122 "\e[38;5;66m"
# define C_123 "\e[38;5;67m"
# define C_124 "\e[38;5;68m"
# define C_125 "\e[38;5;69m"
# define C_130 "\e[38;5;70m"
# define C_131 "\e[38;5;71m"
# define C_132 "\e[38;5;72m"
# define C_133 "\e[38;5;73m"
# define C_134 "\e[38;5;74m"
# define C_135 "\e[38;5;75m"
# define C_140 "\e[38;5;76m"
# define C_141 "\e[38;5;77m"
# define C_142 "\e[38;5;78m"
# define C_143 "\e[38;5;79m"
# define C_144 "\e[38;5;80m"
# define C_145 "\e[38;5;81m"
# define C_150 "\e[38;5;82m"
# define C_151 "\e[38;5;83m"
# define C_152 "\e[38;5;84m"
# define C_153 "\e[38;5;85m"
# define C_154 "\e[38;5;86m"
# define C_155 "\e[38;5;87m"
# define C_200 "\e[38;5;88m"
# define C_201 "\e[38;5;89m"
# define C_202 "\e[38;5;90m"
# define C_203 "\e[38;5;91m"
# define C_204 "\e[38;5;92m"
# define C_205 "\e[38;5;93m"
# define C_210 "\e[38;5;94m"
# define C_211 "\e[38;5;95m"
# define C_212 "\e[38;5;96m"
# define C_213 "\e[38;5;97m"
# define C_214 "\e[38;5;98m"
# define C_215 "\e[38;5;99m"
# define C_220 "\e[38;5;100m"
# define C_221 "\e[38;5;101m"
# define C_222 "\e[38;5;102m"
# define C_223 "\e[38;5;103m"
# define C_224 "\e[38;5;104m"
# define C_225 "\e[38;5;105m"
# define C_230 "\e[38;5;106m"
# define C_231 "\e[38;5;107m"
# define C_232 "\e[38;5;108m"
# define C_233 "\e[38;5;109m"
# define C_234 "\e[38;5;110m"
# define C_235 "\e[38;5;111m"
# define C_240 "\e[38;5;112m"
# define C_241 "\e[38;5;113m"
# define C_242 "\e[38;5;114m"
# define C_243 "\e[38;5;115m"
# define C_244 "\e[38;5;116m"
# define C_245 "\e[38;5;117m"
# define C_250 "\e[38;5;118m"
# define C_251 "\e[38;5;119m"
# define C_252 "\e[38;5;120m"
# define C_253 "\e[38;5;121m"
# define C_254 "\e[38;5;122m"
# define C_255 "\e[38;5;123m"
# define C_300 "\e[38;5;124m"
# define C_301 "\e[38;5;125m"
# define C_302 "\e[38;5;126m"
# define C_303 "\e[38;5;127m"
# define C_304 "\e[38;5;128m"
# define C_305 "\e[38;5;129m"
# define C_310 "\e[38;5;130m"
# define C_311 "\e[38;5;131m"
# define C_312 "\e[38;5;132m"
# define C_313 "\e[38;5;133m"
# define C_314 "\e[38;5;134m"
# define C_315 "\e[38;5;135m"
# define C_320 "\e[38;5;136m"
# define C_321 "\e[38;5;137m"
# define C_322 "\e[38;5;138m"
# define C_323 "\e[38;5;139m"
# define C_324 "\e[38;5;140m"
# define C_325 "\e[38;5;141m"
# define C_330 "\e[38;5;142m"
# define C_331 "\e[38;5;143m"
# define C_332 "\e[38;5;144m"
# define C_333 "\e[38;5;145m"
# define C_334 "\e[38;5;146m"
# define C_335 "\e[38;5;147m"
# define C_340 "\e[38;5;148m"
# define C_341 "\e[38;5;149m"
# define C_342 "\e[38;5;150m"
# define C_343 "\e[38;5;151m"
# define C_344 "\e[38;5;152m"
# define C_345 "\e[38;5;153m"
# define C_350 "\e[38;5;154m"
# define C_351 "\e[38;5;155m"
# define C_352 "\e[38;5;156m"
# define C_353 "\e[38;5;157m"
# define C_354 "\e[38;5;158m"
# define C_355 "\e[38;5;159m"
# define C_400 "\e[38;5;160m"
# define C_401 "\e[38;5;161m"
# define C_402 "\e[38;5;162m"
# define C_403 "\e[38;5;163m"
# define C_404 "\e[38;5;164m"
# define C_405 "\e[38;5;165m"
# define C_410 "\e[38;5;166m"
# define C_411 "\e[38;5;167m"
# define C_412 "\e[38;5;168m"
# define C_413 "\e[38;5;169m"
# define C_414 "\e[38;5;170m"
# define C_415 "\e[38;5;171m"
# define C_420 "\e[38;5;172m"
# define C_421 "\e[38;5;173m"
# define C_422 "\e[38;5;174m"
# define C_423 "\e[38;5;175m"
# define C_424 "\e[38;5;176m"
# define C_425 "\e[38;5;177m"
# define C_430 "\e[38;5;178m"
# define C_431 "\e[38;5;179m"
# define C_432 "\e[38;5;180m"
# define C_433 "\e[38;5;181m"
# define C_434 "\e[38;5;182m"
# define C_435 "\e[38;5;183m"
# define C_440 "\e[38;5;184m"
# define C_441 "\e[38;5;185m"
# define C_442 "\e[38;5;186m"
# define C_443 "\e[38;5;187m"
# define C_444 "\e[38;5;188m"
# define C_445 "\e[38;5;189m"
# define C_450 "\e[38;5;190m"
# define C_451 "\e[38;5;191m"
# define C_452 "\e[38;5;192m"
# define C_453 "\e[38;5;193m"
# define C_454 "\e[38;5;194m"
# define C_455 "\e[38;5;195m"
# define C_500 "\e[38;5;196m"
# define C_501 "\e[38;5;197m"
# define C_502 "\e[38;5;198m"
# define C_503 "\e[38;5;199m"
# define C_504 "\e[38;5;200m"
# define C_505 "\e[38;5;201m"
# define C_510 "\e[38;5;202m"
# define C_511 "\e[38;5;203m"
# define C_512 "\e[38;5;204m"
# define C_513 "\e[38;5;205m"
# define C_514 "\e[38;5;206m"
# define C_515 "\e[38;5;207m"
# define C_520 "\e[38;5;208m"
# define C_521 "\e[38;5;209m"
# define C_522 "\e[38;5;210m"
# define C_523 "\e[38;5;211m"
# define C_524 "\e[38;5;212m"
# define C_525 "\e[38;5;213m"
# define C_530 "\e[38;5;214m"
# define C_531 "\e[38;5;215m"
# define C_532 "\e[38;5;216m"
# define C_533 "\e[38;5;217m"
# define C_534 "\e[38;5;218m"
# define C_535 "\e[38;5;219m"
# define C_540 "\e[38;5;220m"
# define C_541 "\e[38;5;221m"
# define C_542 "\e[38;5;222m"
# define C_543 "\e[38;5;223m"
# define C_544 "\e[38;5;224m"
# define C_545 "\e[38;5;225m"
# define C_550 "\e[38;5;226m"
# define C_551 "\e[38;5;227m"
# define C_552 "\e[38;5;228m"
# define C_553 "\e[38;5;229m"
# define C_554 "\e[38;5;230m"
# define C_555 "\e[38;5;231m"

/*
\e = \x1B = \e = ^[[ = ascii 27

\x[00-ff] write caracter hexa
\[000-377] OCTAL

\e[arg1;arg2;arg3m
COMBINAISONS: BOLD_BLACK "\e[30;1m"

    [nA - Move cursor up n line.
    [nB - Move cursor down n line.
    [nC - Move cursor forward (right) n column.
    [nD - Move cursor backward (left) n column.

    [E - Move cursor to the beginning of the next line.
    [F - Move cursor to the beginning of the previous line.
    [nG - Move cursor to specified column in the current row.
    [n;mH - Move cursor to the position (row and column). [H moves to home
    [J - Clear part of the screen.
        [0J clears from the cursor position to the end of the screen.
        [1J clears from the beginning of the screen to the cursor position.
        [2J clears the entire screen.
    [K - Clear part of the current line.
        [0K clears from the cursor position to the end of the line.
        [1K clears from the beginning of the line to the cursor position.
        [2K clears the entire line.

    [0m: Reset all text formatting (color, style, etc.) to default.
        [1m: Bold or increased intensity.
        [2m: Faint or decreased intensity.
        [3m: Italicized text (not widely supported).
        [4m: Underlined text.
        [5m: Blinking text (not widely supported).
        [7m: Inverted colors (swap foreground and background).
            [8m: Hidden text (not displayed, but still occupies space).
            [9m: Crossed-out or strikethrough text.
                [21m: Reset double underline.
                [22m: Reset bold/bright and dim text to default intensity.
                [24m: Reset underlined text.
                [25m: Reset blinking text.

[0] black [1] red [2] green [3] yellow [4] blue [5] magenta [6] cyan
[7] white) [8] default

"\e[38;5;{foreground_color_code};48;5;{background_color_code}m
    [3(0-8)m: Foreground color [0;3(0-8)m (0; is  the  reset 0)
    [4(0-8)m: Background color [0;4(0-8)m

    [0;38;<n>m: ANSI color codes (0-7) foreground      = [0;38;0;<n>m
    [0;48;<n>m:         Background
    [38;2;<r>;<g>;<b>m: Truecolor foreground (RGB).
    [48;2;<r>;<g>;<b>m:
    [38;5;<n>m: 256-color foreground (0-255).
    [48;5;<n>m:

///////////////////////////////////////////////////////////////////////////////]
    [6n > \e[<row>;<column>R
        used to request the cursor position information. When the terminal
			receives this sequence, it typically responds with another
			sequence containing the current cursor position
    \e[s: Save current cursor position.
        \e[u: Restore cursor position saved with \e[s
    [?25l Hides the cursor.
        [?25h Shows the cursor again.
    [?1049h": Save cursor and screen state (alternative buffer).
        [?1049l": Restore cursor and screen state (main buffer).
    [?2004h": Enable bracketed paste mode (allows the terminal to distinguish
			pasted text from typed text).
        [?2004l": Disable bracketed paste mode.

*/

#endif //COLORS_H

///////////////////////////////////////////////////////////////////////////////]
/*
The color range of a 256 color terminal consists of 4 parts, often 5, in which
	 case you actually get 258 colors:
Color numbers 0 to 7 are the default terminal colors, the actual RGB value of
	which is not standardized and can often be configured.
Color numbers 8 to 15 are the "bright" colors. Most of the time these are a
	lighter shade of the color with index - 8. They are also not standardized
	and can often be configured. Depending on terminal and shell, they are
		often used instead of or in conjunction with bold font faces.
Color numbers 16 to 231 are RGB colors. These 216 colors are defined by 6
	values on each of the three RGB axes. That is, instead of values 0 -
	255, each color only ranges from 0 - 5.
The color number is then calculated like this:
number = 16 + 36 * r + 6 * g + b
with r, g and b in the range 0 - 5.
The color numbers 232 to 255 are grayscale with 24 shades of gray from
	dark to light.
The default colors for foreground and background. In many terminals they can
	be configured independently from the 256 indexed colors, giving an
	additional two configurable colors . You get them when not setting any
	other color or disabling other colors (i.e. print '\e[m').
*/
