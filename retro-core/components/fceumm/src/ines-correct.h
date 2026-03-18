#ifndef _FCEU_INES_CORRECT_H
#define _FCEU_INES_CORRECT_H

	/* ROM images that have the battery-backed bit set in the header that really
	don't have battery-backed RAM is not that big of a problem, so I'll
	treat this differently by only listing games that should have battery-backed RAM.
	*/
	/* CRC32, mapperNum, SubMapper, Mirroring, hasBattery, prgRam, chrRam, region, extra flags */
	{ 0xb17574f3,    1, DEFAULT, DEFAULT, 1,    0x70, DEFAULT, DEFAULT, NOEXTRA }, /* AD&D Heroes of the Lance */
	{ 0x5de61639,    1, DEFAULT, DEFAULT, 1,    0x70,    0x07, DEFAULT, NOEXTRA }, /* AD&D Hillsfar */
	{ 0x2545214c,    1, DEFAULT, DEFAULT, 1,    0x70, DEFAULT, DEFAULT, NOEXTRA }, /* DW */
	{ 0x3b3f88f0,    1, DEFAULT, DEFAULT, 1,    0x70, DEFAULT, DEFAULT, NOEXTRA }, /* DW */
	{ 0x8c5a784e,    1, DEFAULT, DEFAULT, 1,    0x70,    0x07, DEFAULT, NOEXTRA }, /* DW 2 */
	{ 0x506e259d,    1, DEFAULT, DEFAULT, 1,    0x70,    0x07, DEFAULT, NOEXTRA }, /* DW 4*/
	{ 0xa86a5318,    1, DEFAULT, DEFAULT, 1,    0x70,    0x07, DEFAULT, NOEXTRA }, /* Dw 3 */
	{ 0x45f03d2e,    1, DEFAULT, DEFAULT, 1,    0x70, DEFAULT, DEFAULT, NOEXTRA }, /* Faria */
	{ 0xb8b88130,    1, DEFAULT, DEFAULT, 1,    0x70,    0x07, DEFAULT, NOEXTRA }, /* Final Fantasy (FFE Hack) */
	{ 0xcebd2a31,    1, DEFAULT, DEFAULT, 1,    0x70,    0x07, DEFAULT, NOEXTRA }, /* Final Fantasy */
	{ 0xd29db3c7,    1, DEFAULT, DEFAULT, 1,    0x70,    0x07, DEFAULT, NOEXTRA }, /* Final Fantasy 2 */
	{ 0x466efdc2,    1, DEFAULT, DEFAULT, 1,    0x70,    0x07, DEFAULT, NOEXTRA }, /* Final Fantasy J */
	{ 0xeaf7ed72,    1, DEFAULT, DEFAULT, 1,    0x70,    0x07, DEFAULT, NOEXTRA }, /* Legend of Zelda */
	{ 0x3fe272fb,    1, DEFAULT, DEFAULT, 1,    0x70,    0x07, DEFAULT, NOEXTRA }, /* Legend of Zelda */
	{ 0xba322865,    1, DEFAULT, DEFAULT, 1,    0x70, DEFAULT, DEFAULT, NOEXTRA }, /* Zelda 2 */
	{ 0x25952141,    4, DEFAULT, DEFAULT, 1, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* AD&D Pool of Radiance */
	{ 0x1335cb05,    4, DEFAULT, DEFAULT, 1, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* Crystalis */
	{ 0x57e220d0,    4, DEFAULT, DEFAULT, 1, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* Final Fantasy 3 */
	{ 0x889129cb,    4, DEFAULT, DEFAULT, 1, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* Startropics */
	{ 0xd054ffb0,    4, DEFAULT, DEFAULT, 1, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* Startropics 2*/
	{ 0xb5ff71ab,   19, DEFAULT, DEFAULT, 1, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* Battle Fleet */
	{ 0x0c1792da,   19, DEFAULT, DEFAULT, 1, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* Famista '90 */
	{ 0x47c2020b,   19, DEFAULT, DEFAULT, 1, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* Hydlide 3*/
	{ 0xbc11e61a,   19, DEFAULT, DEFAULT, 1, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* Kaijuu Monogatari */
	{ 0xace56f39,   19, DEFAULT, DEFAULT, 1, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* Mindseeker */
	{ 0xe1383deb,   26, DEFAULT, DEFAULT, 1, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* Mouryou Senki Madara */
	{ 0xde9c9c64,   80, DEFAULT, DEFAULT, 1, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* Kyonshiizu 2 */
	{ 0x0e1683c5,   80, DEFAULT, DEFAULT, 1, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* Mirai Shinwa Jarvas */

	{ 0xaf5d7aa2,    0, DEFAULT,    MI_H, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* Clu Clu Land (W) [o3].nes */
	{ 0xcfb224e6,  222, DEFAULT,    MI_H, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* Dragon Ninja (J) (PRG0) [p1][!].nes */
	{ 0xad9c63e2,   70, DEFAULT,    MI_V, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* Space Shadow (Japan).nes */
	{ 0xe1526228,  206, DEFAULT,    MI_V, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* Ki no Bouken - The Quest of Ki (Japan).nes */
	{ 0xaf5d7aa2,    0, DEFAULT,    MI_H, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* Clu Clu Land (W) [o3].nes */
	{ 0xcfb224e6,  222, DEFAULT,    MI_H, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* Dragon Ninja (J) (PRG0) [p1][!].nes */
	{ 0xfcdaca80,    0, DEFAULT,    MI_H, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* Elevator Action (Japan).nes */
	{ 0xc05a365b,    0, DEFAULT,    MI_H, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* Chou Fuyuu Yousai Exed Exes (Japan).nes */
	{ 0xc4c3949a,    0, DEFAULT,    MI_H, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* Mario Bros. (World).nes */
	{ 0x32fa246f,    0, DEFAULT,    MI_H, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* Tag Team Pro-Wrestling (Japan).nes */
	{ 0x43d30c2f,    0, DEFAULT,    MI_H, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* Ms. Pac-Man (USA) (Tengen) (Unl).nes */
	{ 0xb3c30bea,    0, DEFAULT,    MI_H, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* Xevious (Japan) (En).nes */
	{ 0xe492d45a,    0, DEFAULT,    MI_H, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* Zippy Race (Japan).nes */
	{ 0x24598791,    0, DEFAULT,    MI_V, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* Duck Hunt (World).nes */
	{ 0x49aeb3a6,    0, DEFAULT,    MI_V, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* Excitebike (Japan, USA).nes */
	{ 0xb8535ca3,    0, DEFAULT,    MI_V, 1, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* Mahjong (Japan).nes */
	{ 0x330de468,    0, DEFAULT,    MI_V, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* Obake no Q Tarou - Wanwan Panic (Japan).nes */
	{ 0xe28f2596,    0, DEFAULT,    MI_V, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* Pac-Land (J) [b2].nes */
	{ 0x5112dc21,    0, DEFAULT,    MI_V, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* Wild Gunman (World) (Rev 1).nes */
	{ 0xd8ee7669,    1, DEFAULT, DFAULT8, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* Adventures of Rad Gravity, The (USA).nes */
	{ 0x5b837e8d,    1, DEFAULT, DFAULT8, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* Alien Syndrome (Japan).nes */
	{ 0x37ba3261,    1, DEFAULT, DFAULT8, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* Back to the Future Part II & III (USA).nes */
	{ 0x5b6ca654,    1, DEFAULT, DFAULT8, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* Barbie (USA).nes */
	{ 0x61a852ea,    1, DEFAULT, DFAULT8, 1,    0x70, DEFAULT, DEFAULT, NOEXTRA }, /* Battle Stadium - Senbatsu Pro Yakyuu (Japan).nes */
	{ 0xf6fa4453,    1, DEFAULT, DFAULT8, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* Bigfoot (U) [b4].nes */
	{ 0x391aa1b8,    1, DEFAULT, DFAULT8, 1,    0x70,    0x07, DEFAULT, NOEXTRA }, /* Bloody Warriors - Shan-Go no Gyakushuu (Japan).nes */
	{ 0xa5e8d2cd,    1, DEFAULT, DFAULT8, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* BreakThru (USA).nes */
	{ 0x3f56a392,    1, DEFAULT, DFAULT8, 0, DEFAULT,    0x07, DEFAULT, NOEXTRA }, /* Captain ED (Japan).nes */
	{ 0x078ced30,    1, DEFAULT, DFAULT8, 1,    0x07, DEFAULT, DEFAULT, NOEXTRA }, /* Choujin - Ultra Baseball (Japan).nes */
	{ 0xfe364be5,    1, DEFAULT, DFAULT8, 1,    0x70,    0x07, DEFAULT, NOEXTRA }, /* Deep Dungeon IV - Kuro no Youjutsushi (Japan).nes */
	{ 0x57c12280,    1, DEFAULT, DFAULT8, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* Demon Sword (U) [b1].nes */
	{ 0xd09b74dc,    1, DEFAULT, DFAULT8, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* Great Tank (Japan).nes */
	{ 0xe8baa782,    1, DEFAULT, DFAULT8, 1,    0x70, DEFAULT, DEFAULT, NOEXTRA }, /* Gunhed - Aratanaru Tatakai (Japan).nes */
	{ 0x970bd9c2,    1, DEFAULT, DFAULT8, 1,    0x70,    0x07, DEFAULT, NOEXTRA }, /* Hanjuku Eiyuu (J) [b1].nes */
	{ 0xcd7a2fd7,    1, DEFAULT, DFAULT8, 1,    0x70,    0x07, DEFAULT, NOEXTRA }, /* Hanjuku Hero (Japan).nes */
	{ 0x63469396,    1, DEFAULT, DFAULT8, 1,    0x70,    0x07, DEFAULT, NOEXTRA }, /* Hokuto no Ken 4 - Shichisei Haken Den - Hokuto Shinken no Kanata e (Japan).nes */
	{ 0xe94d5181,    1, DEFAULT, DFAULT8, 1,    0x70,    0x07, DEFAULT, NOEXTRA }, /* Future Wars - Mirai Senshi Lios (Japan).nes */
	{ 0x7156cb4d,    1, DEFAULT, DFAULT8, 0,    0x70, DEFAULT, DEFAULT, NOEXTRA }, /* Muppet Adventure - Chaos at the Carnival (USA).nes */
	{ 0x70f67ab7,    1, DEFAULT, DFAULT8, 1,    0x70, DEFAULT, DEFAULT, NOEXTRA }, /* Musashi no Bouken (Japan).nes */
	{ 0x958e4bae,    1, DEFAULT, DFAULT8, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* Orb-3D (USA).nes */
	{ 0x291bcd7d,    1, DEFAULT, DFAULT8, 0, DEFAULT,    0x07, DEFAULT, NOEXTRA }, /* Pachio-kun 2 (Japan).nes */
	{ 0xa9a4ea4c,    1, DEFAULT, DFAULT8, 1,    0x70, DEFAULT, DEFAULT, NOEXTRA }, /* Satomi Hakkenden (J) [b2].nes */
	{ 0xcc3544b0,    1, DEFAULT, DFAULT8, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* Triathron, The (Japan).nes */
	{ 0x52ab2d17,    1, DEFAULT, DFAULT8, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* Toukyou Pachi-Slot Adventure (Japan).nes */
	{ 0x934db14a,    1, DEFAULT, DEFAULT, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* All-Pro Basketball (USA).nes */
	{ 0xf74dfc91,    1, DEFAULT, DEFAULT, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* Win, Lose or Draw (USA).nes */
	{ 0xcfe02ada,    1, DEFAULT, DEFAULT, 0, DEFAULT, DEFAULT,     PAL, NOEXTRA }, /* Darkman (Europe).nes */
	{ 0x1a71fd06,    1, DEFAULT,    MI_H, 0, DEFAULT,    0x07, DEFAULT, NOEXTRA }, /* Kujaku Ou.nes */

	/* MMC1 games with more than 8K wram */
	{ 0x2225c20f,    1, DEFAULT, DEFAULT, 1,    0x77,    0x07, DEFAULT, NOEXTRA }, /* Genghis Khan */
	{ 0xfb69743a,    1, DEFAULT, DEFAULT, 1,    0x77,    0x07, DEFAULT, NOEXTRA }, /* ""        "" (J) */
	{ 0x4642dda6,    1, DEFAULT, DEFAULT, 1,    0x77,    0x07, DEFAULT, NOEXTRA }, /* Nobunaga's Ambition */
	{ 0x3f7ad415,    1, DEFAULT, DEFAULT, 1,    0x77,    0x07, DEFAULT, NOEXTRA }, /* ""        "" (J) (PRG0) */
	{ 0x2b11e0b0,    1, DEFAULT, DEFAULT, 1,    0x77,    0x07, DEFAULT, NOEXTRA }, /* ""        "" (J) (PRG1) */
	{ 0xc6182024,    1, DEFAULT, DEFAULT, 1,    0x77,    0x07, DEFAULT, NOEXTRA }, /* Romance of the 3 Kingdoms */
	{ 0xabbf7217,    1, DEFAULT, DEFAULT, 1,    0x77,    0x07, DEFAULT, NOEXTRA }, /* ""        "" (J) (PRG0) or Sangokushi  */
	{ 0xccf35c02,    1, DEFAULT, DEFAULT, 1,    0x77,    0x07, DEFAULT, NOEXTRA }, /* ""        "" (J) (PRG1) */
	{ 0xb8747abf,    1, DEFAULT, DEFAULT, 1,    0x90,    0x07, DEFAULT, NOEXTRA }, /* Best Play Pro Yakyuu Special (J) (PRG0) */
	{ 0xc3de7c69,    1, DEFAULT, DEFAULT, 1,    0x90,    0x07, DEFAULT, NOEXTRA }, /* ""        "" (J) (PRG1) */
	{ 0xc9556b36,    1, DEFAULT, DEFAULT, 1,    0x90,    0x07, DEFAULT, NOEXTRA }, /* Final Fantasy 1+2 */

	{ 0xe6a477b2,    2, DEFAULT,    MI_V, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* 3-D WorldRunner (USA).nes */
	{ 0x9ea1dc76,    2, DEFAULT,    MI_H, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* Rainbow Islands (USA).nes */
	{ 0x6d65cac6,    2, DEFAULT,    MI_H, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* Terra Cresta (Japan).nes */
	{ 0xe1b260da,    2, DEFAULT,    MI_V, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* Argos no Senshi - Hachamecha Daishingeki (Japan).nes */
	{ 0x1d0f4d6b,    2, DEFAULT,    MI_V, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* Black Bass, The (USA).nes */
	{ 0x266ce198,    2, DEFAULT,    MI_V, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* City Adventure Touch - Mystery of Triangle (J) [b1].nes */
	{ 0x804f898a,    2, DEFAULT,    MI_V, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* Dragon Unit (Japan).nes */
	{ 0x55773880,    2, DEFAULT,    MI_V, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* Adventures of Gilligan's Island, The (USA).nes */
	{ 0x6e0eb43e,    2, DEFAULT,    MI_V, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* Puss 'n Boots - Pero's Great Adventure (USA).nes */
	{ 0x2bb6a0f8,    2, DEFAULT,    MI_V, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* Sherlock Holmes - Hakushaku Reijou Yuukai Jiken (Japan).nes */
	{ 0x28c11d24,    2, DEFAULT,    MI_V, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* Sukeban Deka 3 (J) [b1].nes */
	{ 0x02863604,    2, DEFAULT,    MI_V, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* Sukeban Deka III (Japan).nes */
	{ 0x419461d0,    2, DEFAULT,    MI_V, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* Super Cars (USA).nes */

	/* CNROM by default has bus conflicts disabled, set to submapper 2 to enable bus conflicts */
	{ 0xac8dcdea,    3,       2,    MI_V, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* Cybernoid - The Fighting Machine (USA).nes */

	{ 0x2915faf0,    3,       0,    MI_H, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* Incantation (Asia) (Ja) (Unl).nes */
	{ 0x8f154a0d,    3,       0,    MI_H, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* Pu Ke Jing Ling (Asia) (Unl).nes */
	{ 0xb0c871c5,    3,       0,    MI_V, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* Wei Lai Xiao Zi (Joy Van).nes */
	{ 0xb3be2f71,    3,       0,    MI_H, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* Yanshan Chess (Unl).nes */
	{ 0xd04a40e6,    3,       0,    MI_H, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* Bingo 75 (Asia) (Ja) (Unl).nes */
	{ 0xe41b440f,    3,       0,    MI_H, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* Sidewinder (Joy Van).nes */
	{ 0xebd0644d,    3,       0,    MI_V, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* Master Chu & The Drunkard Hu (Joy Van).nes */
	{ 0xf283cf58,    3,       0,    MI_H, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* Colorful Dragon (Sachen).nes */
	{ 0x2deb12b8,    3,       0,    MI_V, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* Venice Beach Volleyball (Asia) (Ja) (Super Mega) (Unl).nes */

	{ 0xdbf90772,    3, DEFAULT,    MI_H, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* Alpha Mission (USA).nes */
	{ 0xd858033d,    3, DEFAULT,    MI_H, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* ASO - Armored Scrum Object (J).nes */
	{ 0xd858033d,    3, DEFAULT,    MI_H, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* ASO - Armored Scrum Object (J).nes */
	{ 0x637ba508,    3, DEFAULT,    MI_V, 0, DEFAULT, DEFAULT,     PAL, NOEXTRA }, /* Adan y Eva (Spain) (Gluk Video) (Unl).nes */
	{ 0x9bde3267,    3, DEFAULT,    MI_V, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* Adventures of Dino Riki (USA).nes */
	{ 0xd8eff0df,    3, DEFAULT,    MI_V, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* Gradius (Japan).nes */
	{ 0x1d41cc8c,    3, DEFAULT,    MI_V, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* Gyruss (USA).nes */
	{ 0xcf322bb3,    3, DEFAULT,    MI_V, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* John Elway's Quarterback (USA).nes */
	{ 0xb5d28ea2,    3, DEFAULT,    MI_V, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* Mystery Quest (USA).nes */
	{ 0x02cc3973,    3, DEFAULT,    MI_V, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* Ninja Kid (USA).nes */
	{ 0xbc065fc3,    3, DEFAULT,    MI_V, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* Pipe Dream (U) [b1].nes */
	{ 0xc9ee15a7,    3, DEFAULT, DEFAULT, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* Aladdin III (1995) (Unl) [hM03].nes */
	{ 0x8dedea07,    3, DEFAULT,    MI_H, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* Shui Guo Li (Ch) [a1].nes */
	{ 0x684afccd,    3, DEFAULT,    MI_V, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* Space Hunter (Japan).nes */

	{ 0x97b6cb19,    4, DEFAULT, DEFAULT, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* Aladdin (SuperGame) (Mapper 4) [!].nes */
	{ 0xd97c31b0,  206, DEFAULT,    MI_V, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* Lasalle Ishii no Child's Quest (Japan).nes */
	{ 0x404b2e8b,    4, DEFAULT,    MI_4, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* Rad Racer II (USA).nes */
	{ 0x15141401,    4, DEFAULT, DFAULT8, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* Asmik-kun Land (Japan).nes */
	{ 0x4cccd878,    4, DEFAULT, DFAULT8, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* Kyatto Ninden Teyandee (J) [b1].nes */
	{ 0x59280bec,    4, DEFAULT, DFAULT8, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* Jackie Chan (Japan).nes */
	{ 0x7474ac92,    4, DEFAULT, DFAULT8, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* Kabuki - Quantum Fighter (USA).nes */
	{ 0xf2594374,    4, DEFAULT, DFAULT8, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* Matendouji (Japan).nes */
	{ 0x5337f73c,    4, DEFAULT, DFAULT8, 1, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* Niji no Silk Road (Japan).nes */
	{ 0x9eefb4b4,    4, DEFAULT, DFAULT8, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* Pachi-Slot Adventure 2 - Sorotta-kun no Pachi-Slot Tanteidan (Japan).nes */
	{ 0xafe03802,    4, DEFAULT, DFAULT8, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* Pachio-kun 3 (Japan) (Rev A).nes */
	{ 0x21a653c7,    4, DEFAULT, DEFAULT, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* Super Sky Kid (VS).nes */
	{ 0xdb7f07be,    4, DEFAULT, DEFAULT, 1, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* Toki (USA) (Beta).nes */
	{ 0x8F6CC85A,    4, DEFAULT, DEFAULT, 1, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* (KT-220B) Totally Rad 4-in-1.nes [overdump] */
	{ 0xAB9DE91F,    4, DEFAULT, DEFAULT, 1, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* (KT-220B) Totally Rad 4-in-1.nes, Commonly set to mapper 176 */

	/* MMC5 */
	{ 0x671f23a8,    5, DEFAULT, DEFAULT, 0,       0,        0,     PAL, NOEXTRA }, /* Castlevania III - Dracula's Curse (E) */
	{ 0xcd4e7430,    5, DEFAULT, DEFAULT, 0,       0,        0, DEFAULT, NOEXTRA }, /* Castlevania III - Dracula's Curse (KC) */
	{ 0xed2465be,    5, DEFAULT, DEFAULT, 0,       0,        0, DEFAULT, NOEXTRA }, /* Castlevania III - Dracula's Curse (U) */
	{ 0x0afb395e,    5, DEFAULT, DEFAULT, 0,       0,        0, DEFAULT, NOEXTRA }, /* Gun Sight */
	{ 0xb0480ae9,    5, DEFAULT, DEFAULT, 0,       0,        0, DEFAULT, NOEXTRA }, /* Laser Invasion */
	{ 0xb4735fac,    5, DEFAULT, DEFAULT, 0,       0,        0, DEFAULT, NOEXTRA }, /* Metal Slader Glory */
	{ 0xc4fe19f7,    5, DEFAULT, DEFAULT, 0,       0,        0, DEFAULT, NOEXTRA }, /* Metal Slader Glory (English translation) */
	{ 0xbb7f829a,    5, DEFAULT, DEFAULT, 0,       0,        0, DEFAULT, NOEXTRA }, /* Uchuu Keibitai SDF */
	{ 0x0ec6c023,    5, DEFAULT, DEFAULT, 1,    0x70,        0, DEFAULT, NOEXTRA }, /* Gemfire */
	{ 0x9cbadc25,    5, DEFAULT, DEFAULT, 1,    0x70,        0, DEFAULT, NOEXTRA }, /* Just Breed */
	{ 0xbc80fb52,    5, DEFAULT, DEFAULT, 1,    0x70,        0, DEFAULT, NOEXTRA }, /* Royal Blood */
	{ 0xd532e98f,    5, DEFAULT, DEFAULT, 1,    0x70,        0, DEFAULT, NOEXTRA }, /* Shin 4 Nin Uchi Mahjong - Yakuman Tengoku */
	{ 0x15fe6d0f,    5, DEFAULT, DEFAULT, 1,    0x77,        0, DEFAULT, NOEXTRA }, /* Bandit Kings of Ancient China */
	{ 0xfe3488d1,    5, DEFAULT, DEFAULT, 1,    0x77,        0, DEFAULT, NOEXTRA }, /* Daikoukai Jidai */
	{ 0x1ced086f,    5, DEFAULT, DEFAULT, 1,    0x77,        0, DEFAULT, NOEXTRA }, /* Ishin no Arashi */
	{ 0x6396b988,    5, DEFAULT, DEFAULT, 1,    0x77,        0, DEFAULT, NOEXTRA }, /* L'Empereur (J) */
	{ 0x9c18762b,    5, DEFAULT, DEFAULT, 1,    0x77,        0, DEFAULT, NOEXTRA }, /* L'Empereur (U) */
	{ 0xeee9a682,    5, DEFAULT, DEFAULT, 1,    0x77,        0, DEFAULT, NOEXTRA }, /* Nobunaga no Yabou - Sengoku Gunyuu Den (J) (PRG0) */
	{ 0xf9b4240f,    5, DEFAULT, DEFAULT, 1,    0x77,        0, DEFAULT, NOEXTRA }, /* Nobunaga no Yabou - Sengoku Gunyuu Den (J) (PRG1) */
	{ 0x8ce478db,    5, DEFAULT, DEFAULT, 1,    0x77,        0, DEFAULT, NOEXTRA }, /* Nobunaga's Ambition 2 */
	{ 0x39f2ce4b,    5, DEFAULT, DEFAULT, 1,    0x77,        0, DEFAULT, NOEXTRA }, /* Suikoden - Tenmei no Chikai */
	{ 0xaca15643,    5, DEFAULT, DEFAULT, 1,    0x77,        0, DEFAULT, NOEXTRA }, /* Uncharted Waters */
	{ 0x6f4e4312,    5, DEFAULT, DEFAULT, 1,    0x90,        0, DEFAULT, NOEXTRA }, /* Aoki Ookami to Shiroki Mejika - Genchou Hishi */
	{ 0xf540677b,    5, DEFAULT, DEFAULT, 1,    0x90,        0, DEFAULT, NOEXTRA }, /* Nobunaga no Yabou - Bushou Fuuun Roku */
	{ 0xf011e490,    5, DEFAULT, DEFAULT, 1,    0x90,        0, DEFAULT, NOEXTRA }, /* Romance of The Three Kingdoms II */
	{ 0x184c2124,    5, DEFAULT, DEFAULT, 1,    0x90,        0, DEFAULT, NOEXTRA }, /* Sangokushi II (J) (PRG0) */
	{ 0xee8e6553,    5, DEFAULT, DEFAULT, 1,    0x90,        0, DEFAULT, NOEXTRA }, /* Sangokushi II (J) (PRG1) */

	{ 0xf518dd58,    7, DEFAULT, DFAULT8, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* Captain Skyhawk (USA).nes */
	{ 0x6c4a9735,    7, DEFAULT, DFAULT8, 1, DEFAULT, DEFAULT,     PAL, NOEXTRA }, /* WWF Wrestlemania (Europe).nes */
	{ 0x84382231,    9, DEFAULT, DEFAULT, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* Punch-Out!! (Japan) (Gold Edition).nes */
	{ 0xbe939fce,    9, DEFAULT, DEFAULT, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* Punch-Out!! (U) [b1].nes */
	{ 0x7b837fde,    9, DEFAULT, DEFAULT, 1, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* Mike Tyson's Punch-Out!! (PC10) [b1].nes */
	{ 0x345d3a1a,   11, DEFAULT,    MI_V, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* Castle of Deceit (USA) (Unl).nes */
	{ 0xb79f2651,   11, DEFAULT,    MI_V, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* Chiller (USA) (Unl).nes */
	{ 0x5e66eaea,   13, DEFAULT,    MI_V, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* Videomation (USA).nes */
	{ 0xcd373baa,   14, DEFAULT, DEFAULT, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* 武士魂 (8 characters).nes */
	{ 0xbfc7a2e9,   16, DEFAULT, DFAULT8, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* Dragon Ball 3 - Gokuu Den (Japan) (Rev 1).nes */
	{ 0x6e68e31a,   16, DEFAULT, DFAULT8, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* Dragon Ball 3 - Gokuu Den (Japan).nes */
	{ 0x33b899c9,   16, DEFAULT, DEFAULT, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* Dragon Ball - Daimaou Fukkatsu (Japan).nes */
	{ 0xa262a81f,   16, DEFAULT, DEFAULT, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* Rokudenashi Blues (Japan).nes */
	{ 0x286fcd20,   21, DEFAULT, DEFAULT, 1, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* Ganbare Goemon Gaiden 2 - Tenka no Zaihou (Japan).nes */
	{ 0x335e6339,   23, DEFAULT, DEFAULT, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* kung fu legend (unl)[!].nes */
	{ 0x1a8d767b,   23, DEFAULT, DEFAULT, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* kung fu legend (unl)[!p].nes */
	{ 0x0e263d47,   23, DEFAULT, DEFAULT, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* World Hero (Unl) (TV System Select) [!].nes */
	{ 0xe4a291ce,   23, DEFAULT, DEFAULT, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* World Hero (Unl) [!].nes */
	{ 0x51e9cd33,   23, DEFAULT, DEFAULT, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* World Hero (Unl) [b1].nes */
	{ 0x105dd586,   27, DEFAULT, DEFAULT, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* Mi Hun Che (Ch)(full copyrights)[!].nes */
	{ 0xbc9bb6c1,   27, DEFAULT, DEFAULT, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* Super Car (Ch)(full copyrights)[!].nes */
	{ 0x43753886,   27, DEFAULT, DEFAULT, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* Mi Hun Che (Ch)(replaced copyrights)[p1][!].nes */
	{ 0x5b3de3d1,   27, DEFAULT, DEFAULT, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* -- */
	{ 0x511e73f8,   27, DEFAULT, DEFAULT, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* Mi Hun Che (Ch) [p1][b2].nes */
	{ 0x5555fca3,   32, DEFAULT, DFAULT8, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* Ai Sensei no Oshiete - Watashi no Hoshi (J) [b1].nes */
	{ 0x283ad224,   32, DEFAULT, DFAULT8, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* Ai Sensei no Oshiete - Watashi no Hoshi (Japan).nes */
	{ 0x243a8735,   32, DEFAULT,  0x10|4, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* Major League (Japan).nes */
	{ 0x8a7d0abe,   33, DEFAULT, DFAULT8, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* Akira (Japan).nes */
	{ 0x376138d8,   33, DEFAULT, DFAULT8, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* Akira (J) [a1].nes */
	{ 0xadf606f6,   33, DEFAULT, DFAULT8, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* Bakushou!! Jinsei Gekijou (Japan).nes */
	{ 0xbc7b1d0f,   33, DEFAULT, DFAULT8, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* Bakushou!! Jinsei Gekijou 2 (Japan).nes */
	{ 0x7a497ae3,   33, DEFAULT, DFAULT8, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* Don Doko Don (Japan).nes */
	{ 0xbaca10a9,   33, DEFAULT, DFAULT8, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* Golfkko Open (Japan).nes */
	{ 0xf80bdc50,   33, DEFAULT, DFAULT8, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* Insector X (Japan).nes */
	{ 0x2a6559a1,   33, DEFAULT, DFAULT8, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* Operation Wolf (Japan).nes */
	{ 0xaeb7fce9,   33, DEFAULT, DFAULT8, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* Power Blazer (Japan).nes */
	{ 0xd920f9df,   33, DEFAULT, DFAULT8, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* Takeshi no Sengoku Fuuunji (Japan).nes */
	{ 0x3cd4b420,   33, DEFAULT, DFAULT8, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* Takeshi no Sengoku Fuuunji (Japan) (Beta).nes */
	{ 0x4c7c1af3,   34, DEFAULT,    MI_H, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* Caesars Palace (U) [b1].nes */
	{ 0x932ff06e,   34, DEFAULT,    MI_H, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* Classic Concentration (U) [b1].nes */
	{ 0xf46ef39a,   37, DEFAULT, DEFAULT, 0, DEFAULT, DEFAULT,     PAL, NOEXTRA }, /* Super Mario Bros. + Tetris + Nintendo World Cup (Europe) (Rev 1).nes */
	{ 0x4686c5dd,   41, DEFAULT, DEFAULT, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* Caltron - 6 in 1 (USA) (Unl).nes */
	{ 0x090c0c17,   42, DEFAULT,    MI_V, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* Ai Senshi Nicol (FDS Conversion) [p1][!].nes */
	{ 0x4df84825,   42, DEFAULT,    MI_V, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* Ai Senshi Nicol (FDS Conversion) [p2][!].nes */
	{ 0x579e5bc5,   42, DEFAULT,    MI_V, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* Ai Senshi Nicol (FDS Conversion) [p3].nes */
	{ 0xc744f205,   42, DEFAULT,    MI_V, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* Ai Senshi Nicol (FDS Conversion) [p3][t1].nes */
	{ 0x71699765,   42, DEFAULT, DEFAULT, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* Love Warrior Nicol.nes */
	{ 0x6bf3f6a3,   42, DEFAULT, DEFAULT, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* Bio Miracle Bokutte Upa (J) (Mario Baby - FDS Conversion).nes */
	{ 0x5ba1c5cf,   42, DEFAULT, DEFAULT, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* Green Beret (FDS Conversion) (Unl).nes */
	{ 0x50ab1ab2,   42, DEFAULT, DEFAULT, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* ?? Green Beret (FDS Conversion, LH09) (Unl) [U][!][t1] (160K PRG */
	{ 0x7ccb12a3,   43, DEFAULT, DEFAULT, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* ?? SMB2j */
	{ 0x6c71feae,   45, DEFAULT, DEFAULT, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* Kunio 8-in-1 [p1].nes */
	{ 0x40c0ad47,   48, DEFAULT, DFAULT8, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* Flintstones, The - The Rescue of Dino & Hoppy (Japan).nes */
	{ 0xaebd6549,   48, DEFAULT, DFAULT8, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* Bakushou!! Jinsei Gekijou 3 (Japan).nes */
	{ 0x6cdc0cd9,   48, DEFAULT, DFAULT8, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* Bubble Bobble 2 (Japan).nes */
	{ 0x99c395f9,   48, DEFAULT, DFAULT8, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* Captain Saver (Japan).nes */
	{ 0xa7b0536c,   48, DEFAULT, DFAULT8, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* Don Doko Don 2 (Japan).nes */
	{ 0xb17c828a,   48, DEFAULT, DFAULT8, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* Don Doko Don 2 (J) [a1].nes */
	{ 0x40c0ad47,   48, DEFAULT, DFAULT8, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* Flintstones, The - The Rescue of Dino & Hoppy (J).nes */
	{ 0x1500e835,   48, DEFAULT, DFAULT8, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* Jetsons, The - Cogswell's Caper (Japan).nes */
	{ 0xe2c94bc2,   48, DEFAULT, DFAULT8, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* Super Bros 8 (Unl) [!].nes */
	{ 0xa912b064,   51, DEFAULT, DFAULT8, 0, DEFAULT, DEFAULT, DEFAULT,   0x800 }, /* 11-in-1 Ball Games [p1][o1].nes (has CHR ROM when it shouldn't) */
	{ 0x2e72a5d9,   59, DEFAULT, DFAULT8, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* 11-in-1 (66-in-1, 86-in-1, 63-in-1).nes */
	{ 0x39f514fd,   59, DEFAULT, DFAULT8, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* 18 in 1 (118-in-1, 138-in-1, 198-in-1)VTxxxx.nes */
	{ 0xd8b1f465,   59, DEFAULT, DFAULT8, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* 26-in-1 (36-in-1, 46-in-1,56-in-1) VT 335.nes */
	{ 0xcf82fae9,   59, DEFAULT, DFAULT8, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* 28 in 1 (38-in-1, 48-in-1, 58-in1)VTxxxx.nes */
	{ 0xa7a98698,   59, DEFAULT, DFAULT8, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* 28-in-1 (46-in-1, 63-in-1, 118-in-1)VT-5116.nes */
	{ 0x21fd7143,   59, DEFAULT, DFAULT8, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* 41-in-1 (5-in-1,71-in-1)  VT345.nes */
	{ 0x49ec88d6,   59, DEFAULT, DFAULT8, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* 42-in-1 NT-234 Bad Game Road Fighter.nes */
	{ 0x60306f19,   59, DEFAULT, DFAULT8, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* 42-in-1 PCB 3840.nes */
	{ 0x450cd86e,   59, DEFAULT, DFAULT8, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* 48-in-1 (62-in-1,73-in-1,88-in-1) VTxxx.nes */
	{ 0xd774e041,   59, DEFAULT, DFAULT8, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* 50-in-1 (60-in-1,70-in-1,80-in-1) NT-113.nes */
	{ 0x3c4e94f6,   59, DEFAULT, DFAULT8, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* 51-in-1 (61-in-1, 71-in-1, 81-in-1) VT5310.nes */
	{ 0x0422ed44,   59, DEFAULT, DFAULT8, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* 7-in-1 (32-in-1,66-in-1,119-in-1) VT15004.nes */
	{ 0x7efc0d2c,   59, DEFAULT, DFAULT8, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* 7-in-1 (5000-in-1, 999999999-in-1, 10000000-in-1)NC-07N.nes */
	{ 0x6d92dff1,   59, DEFAULT, DFAULT8, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* TN 95-in-1 (6-in-1) [p1].nes */
	{ 0x39ab0fc7,   64, DEFAULT, DFAULT8, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* Hard Drivin' (USA) (Proto) (Unl).nes */
	{ 0xb19a55dd,   64, DEFAULT, DFAULT8, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* Road Runner (USA) (Unl).nes */
	{ 0xf92be3ec,   64, DEFAULT, DEFAULT, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* Rolling Thunder (USA) (Unl).nes */
	{ 0xd114f544,   66, DEFAULT,    MI_H, 1, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* AV Super Real Pachinko (Japan) (Unl).nes */
	{ 0xe84274c5,   66, DEFAULT,    MI_V, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* Mississippi Satsujin Jiken (J) [h2].nes */
	{ 0xbde3ae9b,   66, DEFAULT,    MI_V, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* Doraemon (Japan).nes */
	{ 0x9552e8df,   66, DEFAULT,    MI_V, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* Dragon Ball - Shen Long no Nazo (Japan).nes */
	{ 0x811f06d9,   66, DEFAULT,    MI_V, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* Dragon Power (USA).nes */
	{ 0xd26efd78,   66, DEFAULT,    MI_V, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* Super Mario Bros. + Duck Hunt (USA).nes */
	{ 0xdd8ed0f7,   70, DEFAULT,    MI_V, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* Kamen Rider Club (Japan).nes */
	{ 0xbba58be5,   70, DEFAULT, DEFAULT, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* Family Trainer 6 - Manhattan Police (Japan).nes */
	{ 0x370ceb65,   70, DEFAULT, DEFAULT, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* Family Trainer 5 - Meiro Daisakusen (Japan).nes */
	{ 0x86be4746,    2, DEFAULT, DEFAULT, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* Dooly Bravo Land (Korea) (Unl).nes */
	{ 0xe62e3382,   71, DEFAULT, DEFAULT, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* MiG 29 - Soviet Fighter (USA) (Unl).nes */
	{ 0xac7b0742,   71, DEFAULT, DEFAULT, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* Golden KTV (Ch) [!].nes */
	{ 0x054bd3e9,   74, DEFAULT, DEFAULT, 1, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* Di 4 Ci - Ji Qi Ren Dai Zhan (Ch).nes */
	{ 0x496ac8f7,   74, DEFAULT, DEFAULT, 1, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* Ji Jia Zhan Shi (Ch) [b3].nes */
	{ 0xae854cef,   74, DEFAULT, DEFAULT, 1, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* Jia A Fung Yun (Ch).nes */
	{ 0xba51ac6f,   78, DEFAULT,    MI_4, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* Holy Diver (Japan).nes */
	{ 0x3d1c3137,   78, DEFAULT, DFAULT8, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* Uchuusen Cosmo Carrier (Japan).nes */
	{ 0xa4fbb438,   79, DEFAULT,    MI_H, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* F-15 City War (AVE) (REV1.x) [b1].nes */
	{ 0xd4a76b07,   79, DEFAULT,    MI_H, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* F-15 City War (AVE) (REV1.x) [b2].nes */
	{ 0x8eab381c,   79, DEFAULT,    MI_V, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* Deathbots (USA) (Rev 1) (Unl).nes */
	{ 0x1eb4a920,   79, DEFAULT,    MI_V, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* Double Strike - Aerial Attack Force (USA) (v1.1) (Unl).nes */
	{ 0x3e1271d5,   79, DEFAULT,    MI_V, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* Tiles of Fate (USA) (Unl).nes */
	{ 0xd2699893,   88, DEFAULT,    MI_H, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* Dragon Spirit - Aratanaru Densetsu (Japan).nes */
	{ 0xbb7c5f7a,   89, DEFAULT, DFAULT8, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* Tenka no Goikenban - Mito Koumon (J) [f1].nes */
	{ 0x082778e6,   91,       1,    MI_V, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* Super Fighter III.nes */
	{ 0x10119e6b,   93, DEFAULT, DFAULT8, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* Fantasy Zone (Japan) (Sunsoft).nes */
	{ 0x2b750bf9,  101, DEFAULT,    MI_V, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* Urusei Yatsura - Lum no Wedding Bell (Japan) (Beta).nes */
	{ 0x0da5e32e,  101, DEFAULT,    MI_V, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* Urusei Yatsura - Lum no Wedding Bell (Japan).nes */
	{ 0x6096f84e,  104, DEFAULT,    MI_V, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* Pegasus 5-in-1 (Golden Five) (Unl).nes */
	{ 0x3d3ff543,  113, DEFAULT,    MI_H, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* Kazama Jun to Asama Yuuko no AV Dragon Mahjong (Japan) (Unl).nes */
	{ 0x68379fdb,  113, DEFAULT,    MI_V, 1, DEFAULT, DEFAULT,     PAL, NOEXTRA }, /* Pipemania (Australia) (HES) (Unl).nes */
	{ 0x6a03d3f3,  114, DEFAULT, DEFAULT, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* Lion King, The (Unl) (Mapper 114).nes */
	{ 0x0d98db53,  114, DEFAULT, DEFAULT, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* Pocahontas (Unl).nes */
	{ 0xf5676f0b,  114, DEFAULT, DEFAULT, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* Super Donkey Kong (Unl) [b1].nes */
	{ 0xc5e5c5b2,  115, DEFAULT, DEFAULT, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* Bao Qing Tian (Ch).nes */
	{ 0xe40dfb7e,  116, DEFAULT, DEFAULT, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* Somari (SOMARI-P) (NT-616) (Unl) [!].nes */
	{ 0xc9371ebb,  116, DEFAULT, DEFAULT, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* Somari (SOMARI-W) (Unl) [!].nes */
	{ 0x78b657ac,  118, DEFAULT, DEFAULT, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* Armadillo (Japan).nes */
	{ 0x90c773c1,  118, DEFAULT, DEFAULT, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* Goal! Two (USA).nes */
	{ 0xb9b4d9e0,  118, DEFAULT, DEFAULT, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* NES Play Action Football (USA).nes */
	{ 0x07d92c31,  118, DEFAULT, DEFAULT, 1, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* RPG Jinsei Game (Japan).nes */
	{ 0x37b62d04,  118, DEFAULT, DEFAULT, 1, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* Ys III - Wanderers from Ys (Japan).nes */
	{ 0x318e5502,  121, DEFAULT, DEFAULT, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* Sonic 3D Blast 6 (Unl).nes */
	{ 0xddcfb058,  121, DEFAULT, DEFAULT, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* Street Fighter Zero 2 '97 (Unl) [!].nes */
	{ 0xd2674b0a,  132, DEFAULT,    MI_H, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* Qi Wang - Chinese Chess (Asia) (Ja) (Unl).nes */
	{ 0x5aefbc94,  133, DEFAULT, DEFAULT, 0, DEFAULT, DEFAULT,     PAL, NOEXTRA }, /* Jovial Race (Asia) (Ja) (PAL) (Unl).nes */
	{ 0xB550B627,  136, DEFAULT, DEFAULT, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* Incantation (Dip Bin) (Joy Van).nes */
	{ 0xc2df0a00,  140, DEFAULT,    MI_V, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* Bio Senshi Dan - Increaser Tono Tatakai (J) [hM66][b3].nes */
	{ 0xe46b1c5d,  140, DEFAULT,    MI_V, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* Mississippi Satsujin Jiken (Japan).nes */
	{ 0x3293afea,  140, DEFAULT,    MI_V, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* Mississippi Satsujin Jiken (Japan) (Rev A).nes */
	{ 0x6bc65d7e,  140, DEFAULT,    MI_V, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* Youkai Club (Japan).nes */
	{ 0x5caa3e61,  144, DEFAULT,    MI_V, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* Death Race (USA) (Unl).nes */
	{ 0x48239b42,  146, DEFAULT, DEFAULT, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* Mahjang Companion (Asia) (Ja) (Hacker) (Unl).nes */
	{ 0xb6a727fa,  146, DEFAULT, DEFAULT, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* Papillon Gals (Japan) (Unl).nes */
	{ 0xa62b79e1,  146, DEFAULT, DEFAULT, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* Sidewinder (HES) [o1].nes */
	{ 0xcc868d4e,  149, DEFAULT, DEFAULT, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* Taiwan Mahjong 16 (Sachen) [a1][!].nes */
	{ 0x29582ca1,  150, DEFAULT, DEFAULT, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* Mei Nu Quan (Honey Peach) (Sachen) [!].nes */
	{ 0x40dbf7a2,  150, DEFAULT, DEFAULT, 0, DEFAULT, DEFAULT,     PAL, NOEXTRA }, /* Olympic IQ (Asia) (Ja) (PAL) (Unl).nes */
	{ 0x73fb55ac,  150, DEFAULT, DEFAULT, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* Lightgun Game 2 in 1 - Cosmocop + Cyber Monster (Asia) (Ja) (Unl).nes */
	{ 0xddcbda16,  150, DEFAULT, DEFAULT, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* Lightgun Game 2 in 1 - Tough Cop + Super Tough Cop (Asia) (Ja) (Unl).nes */
	{ 0x47918d84,  150, DEFAULT, DEFAULT, 0, DEFAULT, DEFAULT,     PAL, NOEXTRA }, /* Auto-Upturn (Asia) (Ja) (PAL) (Unl).nes */
	{ 0x471173e7,  150, DEFAULT, DEFAULT, 0, DEFAULT, DEFAULT,     PAL, NOEXTRA }, /* Chinese Checkers (Asia) (Ja) (PAL) (Unl).nes */
	{ 0x2394ae1c,  150, DEFAULT, DEFAULT, 0, DEFAULT, DEFAULT,     PAL, NOEXTRA }, /* Happy Pairs (Asia) (Ja) (PAL) (Unl).nes */
	{ 0xcab40a6c,  150, DEFAULT, DEFAULT, 0, DEFAULT, DEFAULT,     PAL, NOEXTRA }, /* Magic Cube (Asia) (Ja) (PAL) (Unl).nes */
	{ 0xbe17e27b,  150, DEFAULT, DEFAULT, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* Poker III (Asia) (Ja) (Alt 2) (Unl).nes */
	{ 0x34ddf806,  150, DEFAULT, DEFAULT, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* Strategist (Asia) (Ja) (NTSC) (Unl).nes */
	{ 0xc06facfc,  150, DEFAULT, DEFAULT, 0, DEFAULT, DEFAULT,     PAL, NOEXTRA }, /* Strategist (Asia) (Ja) (PAL) (Unl).nes */
	{ 0xa95a915a,  150, DEFAULT, DEFAULT, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* Tasac (Asia) (Ja) (Unl).nes */
	{ 0x0f141525,  152, DEFAULT, DFAULT8, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* Arkanoid II (Japan).nes */
	{ 0xbda8f8e4,  152, DEFAULT, DFAULT8, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* Gegege no Kitarou 2 - Youkai Gundan no Chousen (Japan).nes */
	{ 0xb1a94b82,  152, DEFAULT, DFAULT8, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* Pocket Zaurus - Juu Ouken no Nazo (Japan).nes */
	{ 0x026c5fca,  152, DEFAULT, DFAULT8, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* Saint Seiya - Ougon Densetsu (Japan).nes */
	{ 0x3f15d20d,  153, DEFAULT, DFAULT8, 1, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* Famicom Jump II - Saikyou no 7 Nin (Japan).nes */
	{ 0xd1691028,  154, DEFAULT, DFAULT8, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* Devil Man (Japan).nes */
	{ 0xcfd4a281,  155, DEFAULT, DFAULT8, 1, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* Money Game, The (Japan).nes */
	{ 0x2f27cdef,  155, DEFAULT, DFAULT8, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* Tatakae!! Rahmen Man - Sakuretsu Choujin 102 Gei (J) [b1].nes */
	{ 0xc1719664,  155, DEFAULT, DFAULT8, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* Tatakae!! Rahmen Man - Sakuretsu Choujin 102 Gei (Japan).nes */
	{ 0xccc03440,  156, DEFAULT, DEFAULT, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* Buzz & Waldog (USA) (Proto) (Unl).nes */
	{ 0x983d8175,  157, DEFAULT, DFAULT8, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* Datach - Battle Rush - Build Up Robot Tournament (Japan).nes */
	{ 0x894efdbc,  157, DEFAULT, DFAULT8, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* Datach - Crayon Shin-chan - Ora to Poi Poi (Japan).nes */
	{ 0x19e81461,  157, DEFAULT, DFAULT8, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* Datach - Dragon Ball Z - Gekitou Tenkaichi Budoukai (Japan).nes */
	{ 0xbe06853f,  157, DEFAULT, DFAULT8, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* Datach - J.League Super Top Players (Japan).nes */
	{ 0x0be0a328,  157, DEFAULT, DFAULT8, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* Datach - SD Gundam - Gundam Wars (Japan).nes */
	{ 0x5b457641,  157, DEFAULT, DFAULT8, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* Datach - Ultraman Club - Supokon Fight! (Japan).nes */
	{ 0xf51a7f46,  157, DEFAULT, DFAULT8, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* Datach - Yu Yu Hakusho - Bakutou Ankoku Bujutsukai (Japan).nes */
	{ 0xcbf4366f,  158, DEFAULT, DFAULT8, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* Alien Syndrome (USA) (Unl).nes */
	{ 0xe170404c,  159, DEFAULT, DEFAULT, 1,    0x10, DEFAULT, DEFAULT, NOEXTRA }, /* SD Gundam Gaiden - Knight Gundam Monogatari (Japan).nes */
	{ 0x276ac722,  159, DEFAULT, DEFAULT, 1,    0x10, DEFAULT, DEFAULT, NOEXTRA }, /* SD Gundam Gaiden - Knight Gundam Monogatari (Japan) (Rev 1).nes */
	{ 0x0cf42e69,  159, DEFAULT, DEFAULT, 1,    0x10, DEFAULT, DEFAULT, NOEXTRA }, /* Magical Taruruuto-kun - Fantastic World!! (Japan).nes */
	{ 0xdcb972ce,  159, DEFAULT, DEFAULT, 1,    0x10, DEFAULT, DEFAULT, NOEXTRA }, /* Magical Taruruuto-kun - Fantastic World!! (Japan) (Rev 1).nes */
	{ 0xb7f28915,  159, DEFAULT, DEFAULT, 1,    0x10, DEFAULT, DEFAULT, NOEXTRA }, /* Magical Taruruuto-kun 2 - Mahou Daibouken (Japan).nes */
	{ 0x183859d2,  159, DEFAULT, DEFAULT, 1,    0x10, DEFAULT, DEFAULT, NOEXTRA }, /* Dragon Ball Z - Kyoushuu! Saiya Jin (Japan).nes */
	{ 0x58152b42,  160, DEFAULT,    MI_V, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* Pipe V (Asia) (Ja) (Unl).nes */

	/* Mapper 162: Waixing FS304 */
	{ 0x08FBF3F0,  162,       0,    MI_H, 1,    0x70,    0x07,   DENDY, NOEXTRA }, /* Mummy - 神鬼传奇 */
	{ 0xF5D34C8E,  162,       0,    MI_H, 1,    0x70,    0x07,   DENDY, NOEXTRA }, /* Zelda 传说: 三神之力 */
	{ 0xEE3A1CA8,  162,       0,    MI_H, 1,    0x70,    0x07,   DENDY, NOEXTRA }, /* 法老王 - Pharaoh */
	{ 0xCF4ADAAD,  162,       0,    MI_H, 1,    0x70,    0x07,   DENDY, NOEXTRA }, /* 火焰纹章 - 圣战的系谱 */
	{ 0xB2045E9C,  162,       0,    MI_H, 1,    0x70,    0x07,   DENDY, NOEXTRA }, /* 聖火徽章 III */
	{ 0x1C098942,  162,       0,    MI_H, 1,    0x70,    0x07,   DENDY, NOEXTRA }, /* 西游记后传 */
	/* Mapper 162: Nanjing games that use an FS304 feature as a protection check */
	{ 0x8589652D,  162,       0,    MI_V, 1,    0x70,    0x07,   DENDY, NOEXTRA }, /* 农场小精灵 */
	{ 0x99FE9AB5,  162,       0,    MI_V, 1,    0x70,    0x07,   DENDY, NOEXTRA }, /* 时空斗士 - Pegasus Senya */
	{ 0x82F204AE,  162,       0,    MI_V, 1,    0x70,    0x07,   DENDY, NOEXTRA }, /* 梁山英雄 */
	
	/* Mapper 163: Nanjing, running with $5300=$04 */
	{ 0xC9ABA7F0,  163,       0,    MI_V, 1,    0x70,    0x07,   DENDY, NOEXTRA }, /* Chrono Trigger - 时空之轮 */
	{ 0x143B4D30,  163,       0,    MI_V, 1,    0x70,    0x07,   DENDY, NOEXTRA }, /* Final Fantasy VII - 最终幻想 7 */
	{ 0x609458B6,  163,       0,    MI_V, 1,    0x70,    0x07,   DENDY, NOEXTRA }, /* Final Fantasy VII - 最终幻想 7, English translation */
	{ 0x3CC55A44,  163,       0,    MI_V, 1,    0x70,    0x07,   DENDY, NOEXTRA }, /* The Legend of Zelda - 塞尔达传说: 神奇的帽子 */
	{ 0x4752BD5E,  163,       0,    MI_V, 1,    0x70,    0x07,   DENDY, NOEXTRA }, /* 七國大戰 */
	{ 0x1121C0D1,  163,       0,    MI_V, 1,    0x70,    0x07,   DENDY, NOEXTRA }, /* 三国志 之 傲视天地 */
	{ 0x8E4294A9,  163,       0,    MI_V, 1,    0x70,    0x07,   DENDY, NOEXTRA }, /* 三国志: 曹操传 */
	{ 0xF52468E7,  163,       0,    MI_V, 1,    0x70,    0x07,   DENDY, NOEXTRA }, /* 三国无双: 猛将传 */
	{ 0xEF7BA485,  163,       0,    MI_V, 1,    0x70,    0x07,   DENDY, NOEXTRA }, /* 仙剑奇侠 */
	{ 0x20E1CF44,  163,       0,    MI_V, 1,    0x70,    0x07,   DENDY, NOEXTRA }, /* 仙界精灵 */
	{ 0x9688AEEA,  163,       0,    MI_V, 1,    0x70,    0x07,   DENDY, NOEXTRA }, /* 倚天传说 */
	{ 0x04DFE0D4,  163,       0,    MI_V, 1,    0x70,    0x07,   DENDY, NOEXTRA }, /* 倚天剑传奇 */
	{ 0x6CB6D619,  163,       0,    MI_V, 1,    0x70,    0x07,   DENDY, NOEXTRA }, /* 口袋宝石 - 银 */
	{ 0x8C73A47B,  163,       0,    MI_V, 1,    0x70,    0x07,   DENDY, NOEXTRA }, /* 哪吒传奇 */
	{ 0xE40DA18F,  163,       0,    MI_V, 1,    0x70,    0x07,   DENDY, NOEXTRA }, /* 圣剑传说 */
	{ 0xC07CD2CE,  163,       0,    MI_V, 1,    0x70,    0x07,   DENDY, NOEXTRA }, /* 圣斗士星矢: 天马之幻想 */
	{ 0x9237C200,  163,       0,    MI_V, 1,    0x70,    0x07,   DENDY, NOEXTRA }, /* 大话西游 */
	{ 0x4FB02A43,  163,       0,    MI_V, 1,    0x70,    0x07,   DENDY, NOEXTRA }, /* 天龙八部 */
	{ 0x52D7FE18,  163,       0,    MI_V, 1,    0x70,    0x07,   DENDY, NOEXTRA }, /* 太空幻想 */
	{ 0xA01CA587,  163,       0,    MI_V, 1,    0x70,    0x07,   DENDY, NOEXTRA }, /* 幻世录 */
	{ 0x89F4ACD1,  163,       0,    MI_V, 1,    0x70,    0x07,   DENDY, NOEXTRA }, /* 幻想传说 */
	{ 0x65C63CC3,  163,       0,    MI_V, 1,    0x70,    0x07,   DENDY, NOEXTRA }, /* 数码战队 2 */
	{ 0x2C01DE06,  163,       0,    MI_V, 1,    0x70,    0x07,   DENDY, NOEXTRA }, /* 数码暴龙 */
	{ 0xB614AAA2,  163,       0,    MI_V, 1,    0x70,    0x07,   DENDY, NOEXTRA }, /* 暗黑破坏神 - Diablo */
	{ 0x80A18CDC,  163,       0,    MI_V, 1,    0x70,    0x07,   DENDY, NOEXTRA }, /* 机器人大战 */
	{ 0x33DB45BA,  163,       0,    MI_V, 1,    0x70,    0x07,   DENDY, NOEXTRA }, /* 核心危机 */
	{ 0x1B74A022,  163,       0,    MI_V, 1,    0x70,    0x07,   DENDY, NOEXTRA }, /* 梦幻沙漏 */
	{ 0x695A7A70,  163,       0,    MI_V, 1,    0x70,    0x07,   DENDY, NOEXTRA }, /* 楚汉风云 */
	{ 0xD0807FD2,  163,       0,    MI_V, 1,    0x70,    0x07,   DENDY, NOEXTRA }, /* 水浒 III */
	{ 0x524AF6E8,  163,       0,    MI_V, 1,    0x70,    0x07,   DENDY, NOEXTRA }, /* 水浒神兽 (南晶) */
	{ 0x2802E40F,  163,       0,    MI_V, 1,    0x70,    0x07,   DENDY, NOEXTRA }, /* 汉刘邦 */
	{ 0xAC491507,  163,       0,    MI_V, 1,    0x70,    0x07,   DENDY, NOEXTRA }, /* 游戏王 */
	{ 0x31C1BF98,  163,       0,    MI_V, 1,    0x70,    0x07,   DENDY, NOEXTRA }, /* 牧场物语 */
	{ 0x191F7D5E,  163,       0,    MI_V, 1,    0x70,    0x07,   DENDY, NOEXTRA }, /* 真 Samurai Spirits - 武士道列传: 侍魂 */
	{ 0x1FE67BB3,  163,       0,    MI_V, 1,    0x70,    0x07,   DENDY, NOEXTRA }, /* 真 Samurai Spirits - 魂之利刃 */
	{ 0x4973B16B,  163,       0,    MI_V, 1,    0x70,    0x07,   DENDY, NOEXTRA }, /* 石器时代 */
	{ 0x09C7AED3,  163,       0,    MI_V, 1,    0x70,    0x07,   DENDY, NOEXTRA }, /* 葫芦金刚 */
	{ 0x4E3EDF88,  163,       0,    MI_V, 1,    0x70,    0x07,   DENDY, NOEXTRA }, /* 超级机器人大战 A */
	{ 0x74C1EDC7,  163,       0,    MI_V, 1,    0x70,    0x07,   DENDY, NOEXTRA }, /* 轩辕剑外传 之 天之痕 */
	{ 0x723C6345,  163,       0,    MI_V, 1,    0x70,    0x07,   DENDY, NOEXTRA }, /* 金庸群侠传: 书剑江山 */
	{ 0xDA47B05A,  163,       0,    MI_V, 1,    0x70,    0x07,   DENDY, NOEXTRA }, /* 隋唐英雄 */
	{ 0x056F2B8E,  163,       0,    MI_V, 1,    0x70,    0x07,   DENDY, NOEXTRA }, /* 雷电皇: Pikachu传说 */
	{ 0x04166E96,  163,       0,    MI_V, 1,    0x70,    0x07,   DENDY, NOEXTRA }, /* 魔幻世界 */
	{ 0x85FA53E1,  163,       0,    MI_V, 1,    0x70,    0x07,   DENDY, NOEXTRA }, /* 魔界塔士 */
	{ 0x2121DAB2,  163,       0,    MI_V, 1,    0x70,    0x07,   DENDY, NOEXTRA }, /* 魔界霸主 */
	{ 0xE3EF9739,  163,       0,    MI_V, 1,    0x70,    0x07,   DENDY, NOEXTRA }, /* 黄金太阳 */	
	/* Mapper 163: Nanjing, running with $5300=$07 */
	{ 0x6F94C5E5,  163,       0,    MI_V, 1,    0x70,    0x07,   DENDY, NOEXTRA }, /* Final Fantasy IV - 最终幻想 4: 光与暗 水晶纷争 */
	{ 0xD6CBB05D,  163,       0,    MI_V, 1,    0x70,    0x07,   DENDY, NOEXTRA }, /* Naruto RPG - 火影忍者 */
	{ 0xD7A4CBA5,  163,       0,    MI_V, 1,    0x70,    0x07,   DENDY, NOEXTRA }, /* 七龙珠大冒险 - Dragon Ball */
	{ 0xDF27B96C,  163,       0,    MI_V, 1,    0x70,    0x07,   DENDY, NOEXTRA }, /* 三国大乱斗: 战国 - Orochi */
	{ 0x8C60CECF,  163,       0,    MI_V, 1,    0x70,    0x07,   DENDY, NOEXTRA }, /* 三国志: 吕布传 */
	{ 0x44AC9C8E,  163,       0,    MI_V, 1,    0x70,    0x07,   DENDY, NOEXTRA }, /* 三国群侠传 */
	{ 0xA3193C51,  163,       0,    MI_V, 1,    0x70,    0x07,   DENDY, NOEXTRA }, /* 倚天屠龙记 */
	{ 0x98266D3A,  163,       0,    MI_V, 1,    0x70,    0x07,   DENDY, NOEXTRA }, /* 傲视三国志 */
	{ 0xA026AE52,  163,       0,    MI_V, 1,    0x70,    0x07,   DENDY, NOEXTRA }, /* 口袋宝石 - 红 */
	{ 0xB6F72A18,  163,       0,    MI_V, 1,    0x70,    0x07,   DENDY, NOEXTRA }, /* 口袋宝石 - 蓝 */
	{ 0x5108AB7F,  163,       0,    MI_V, 1,    0x70,    0x07,   DENDY, NOEXTRA }, /* 口袋宝石 - 金 */
	{ 0x8EB1B4CF,  163,       0,    MI_V, 1,    0x70,    0x07,   DENDY, NOEXTRA }, /* 口袋水晶 */
	{ 0xA66756AD,  163,       0,    MI_V, 1,    0x70,    0x07,   DENDY, NOEXTRA }, /* 口袋钻石 */
	{ 0x3A613060,  163,       0,    MI_V, 1,    0x70,    0x07,   DENDY, NOEXTRA }, /* 吞食天地 VI: 刘备传 */
	{ 0xB35BE92F,  163,       0,    MI_V, 1,    0x70,    0x07,   DENDY, NOEXTRA }, /* 圣斗士星矢: 北欧突击篇 */
	{ 0x15E50ECD,  163,       0,    MI_V, 1,    0x70,    0x07,   DENDY, NOEXTRA }, /* 宠物高达战记 */
	{ 0xBBAB3A61,  163,       0,    MI_V, 1,    0x70,    0x07,   DENDY, NOEXTRA }, /* 拳皇R-1: 最强格斗王 */
	{ 0x2DA3A49C,  163,       0,    MI_V, 1,    0x70,    0x07,   DENDY, NOEXTRA }, /* 时空斗士 - Pegasus Senya, turns out to be wrong after all, and the mapper 162 one is the correct one */
	{ 0x222A136A,  163,       0,    MI_V, 1,    0x70,    0x07,   DENDY, NOEXTRA }, /* 武侠天地 */
	{ 0xEBC6E2E2,  163,       0,    MI_V, 1,    0x70,    0x07,   DENDY, NOEXTRA }, /* 毁灭之神 */
	{ 0xE08AB52E,  163,       0,    MI_V, 1,    0x70,    0x07,   DENDY, NOEXTRA }, /* 魔兽世界: 恶魔猎人 */
	/* Mapper 162: Nanjing, running with $5300=$07, but dumped without taking the D0/D1 bit swap into account.
	               Such dumps need not be invalidated however, as they can run as mapper 162 when properly emulated. */
	{ 0x9D8AA034,  162,       0,    MI_V, 1,    0x70,    0x07,   DENDY, NOEXTRA }, /* Final Fantasy IV - 最终幻想4: 光与暗 水晶纷争 */
	{ 0x5E66E6C4,  162,       0,    MI_V, 1,    0x70,    0x07,   DENDY, NOEXTRA }, /* Naruto RPG - 火影忍者 */
	{ 0x9DE10A91,  162,       0,    MI_V, 1,    0x70,    0x07,   DENDY, NOEXTRA }, /* 七龙珠大冒险 - Dragon Ball */
	{ 0xFC209609,  162,       0,    MI_V, 1,    0x70,    0x07,   DENDY, NOEXTRA }, /* 三国大乱斗: 战国 - Orochi */
	{ 0x696D98E3,  162,       0,    MI_V, 1,    0x70,    0x07,   DENDY, NOEXTRA }, /* 三国志: 吕布传 */
	{ 0x9F197F2B,  162,       0,    MI_V, 1,    0x70,    0x07,   DENDY, NOEXTRA }, /* 三国群侠传 */
	{ 0x975F64E2,  162,       0,    MI_V, 1,    0x70,    0x07,   DENDY, NOEXTRA }, /* 倚天屠龙记 */
	{ 0x915C5179,  162,       0,    MI_V, 1,    0x70,    0x07,   DENDY, NOEXTRA }, /* 傲视三国志 */
	{ 0x852BDB36,  162,       0,    MI_V, 1,    0x70,    0x07,   DENDY, NOEXTRA }, /* 口袋宝石 - 红 */
	{ 0xB41CF445,  162,       0,    MI_V, 1,    0x70,    0x07,   DENDY, NOEXTRA }, /* 口袋宝石 - 蓝 */
	{ 0x7829C3A9,  162,       0,    MI_V, 1,    0x70,    0x07,   DENDY, NOEXTRA }, /* 口袋宝石 - 金 */
	{ 0xBC383C09,  162,       0,    MI_V, 1,    0x70,    0x07,   DENDY, NOEXTRA }, /* 口袋水晶 */
	{ 0xA9C4712A,  162,       0,    MI_V, 1,    0x70,    0x07,   DENDY, NOEXTRA }, /* 口袋钻石 */
	{ 0xC2B02B71,  162,       0,    MI_V, 1,    0x70,    0x07,   DENDY, NOEXTRA }, /* 吞食天地 VI: 刘备传 */
	{ 0x054444A0,  162,       0,    MI_V, 1,    0x70,    0x07,   DENDY, NOEXTRA }, /* 圣斗士星矢: 北欧突击篇 */
	{ 0x9BA518BA,  162,       0,    MI_V, 1,    0x70,    0x07,   DENDY, NOEXTRA }, /* 宠物高达战记 */
	{ 0x4CE082F8,  162,       0,    MI_V, 1,    0x70,    0x07,   DENDY, NOEXTRA }, /* 拳皇R-1: 最强格斗王 */
	{ 0x57414FB6,  162,       0,    MI_V, 1,    0x70,    0x07,   DENDY, NOEXTRA }, /* 武侠天地 */
	{ 0x979239DE,  162,       0,    MI_V, 1,    0x70,    0x07,   DENDY, NOEXTRA }, /* 毁灭之神 */
	{ 0xEFF96E8A,  162,       0,    MI_V, 1,    0x70,    0x07,   DENDY, NOEXTRA }, /* 魔兽世界: 恶魔猎人 */
	
	/* Mapper 164: Dongda PEC-9588 and Yancheng cy2000-3 PCB */
	{ 0xFE31765B,  164,       0,    MI_V, 0,    0x07,    0x07,   DENDY, NOEXTRA }, /* Pyramid 金字塔 PEC-9588 家庭电脑 */
	{ 0x0878A7EE,  164,       0,    MI_H, 1,    0x35,    0x07,   DENDY, NOEXTRA }, /* Dark Seed - 黑暗之蛊 */
	{ 0x56A0D271,  164,       0,    MI_H, 1,    0x35,    0x07,   DENDY, NOEXTRA }, /* Final Fantasy 太空戰士 V (rev0) */
	{ 0xCB1EF911,  164,       0,    MI_H, 1,    0x35,    0x07,   DENDY, NOEXTRA }, /* Final Fantasy 太空戰士 V (rev1) */
	{ 0x8209BA79,  164,       0,    MI_H, 1,    0x35,    0x07,   DENDY, NOEXTRA }, /* 櫻桃小丸子 */
	{ 0xBC7562A6,  164,       0,    MI_H, 1,    0x35,    0x07,   DENDY, NOEXTRA }, /* 口袋精靈: 金 */
	{ 0x65F1DB91,  164,       0,    MI_H, 1,    0x35,    0x07,   DENDY, NOEXTRA }, /* 大話西游 [restored, no good dump known] */
	{ 0x0A244228,  164,       0,    MI_H, 1,    0x35,    0x07,   DENDY, NOEXTRA }, /* 岳飛傳 [restored, no good dump known] */

	/* Mapper 558: Waixing FS??? and Yancheng YC-03-09 PCB */
	{ 0xE65A8C08,  558,       0,    MI_H, 1,    0x30,    0x07,   DENDY, NOEXTRA }, /* 大話三國 */
	{ 0x228559A7,  558,       0,    MI_H, 1,    0x30,    0x07,   DENDY, NOEXTRA }, /* 口袋精靈: 水晶 */
	{ 0x8B41D49C,  558,       0,    MI_H, 1,    0x70,    0x07,   DENDY, NOEXTRA }, /* Pet Evolve - 宠物进化史 */
	{ 0x5622EC51,  558,       0,    MI_V, 1,    0x70,    0x07,   DENDY, NOEXTRA }, /* 三国志 II 代 */
	{ 0xE7AEB114,  558,       0,    MI_H, 1,    0x70,    0x07,   DENDY, NOEXTRA }, /* 口袋妖怪: 鑽石版 */
	{ 0x8EB4BB51,  558,       0,    MI_H, 1,    0x70,    0x07,   DENDY, NOEXTRA }, /* 口袋精靈: 紅 */
	{ 0x48244391,  558,       0,    MI_H, 1,    0x70,    0x07,   DENDY, NOEXTRA }, /* 宠物: 红 */
	{ 0x01A24301,  558,       0,    MI_H, 1,    0x70,    0x07,   DENDY, NOEXTRA }, /* 数码宝贝 */
	{ 0x88CB68A7,  558,       0,    MI_H, 1,    0x70,    0x07,   DENDY, NOEXTRA }, /* 数码暴龙 4: 水晶版 */
	{ 0x081CAAFF,  558,       0,    MI_H, 1,    0x70,    0x07,   DENDY, NOEXTRA }, /* 盟军敢死队 - Commandos */

	/* Mapper 176 */
	{ 0xffde0de5,  176, DEFAULT, DEFAULT, 1, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* 梦幻之星4[简体](修正)一战一级.nes */
	{ 0xE0ED68B1,  176,       0, DEFAULT, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* (AA-6109) 4-in-1.nes */
	{ 0xC447B9FE,  176,       0, DEFAULT, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* (BRC-4) Super 4-in-1.nes */
	{ 0x922D16FD,  176,       0, DEFAULT, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* (FK-9087) Super Game 3-in-1.nes */
	{ 0xC18A7BCB,  176,       0, DEFAULT, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* (KT-443B) 4-in-1.nes */
	{ 0x43BDB0FF,  176,       0, DEFAULT, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* (KT-8095) Super Game 4-in-1.nes */
	{ 0x719CCE0A,  176,       0, DEFAULT, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* (KT-8394) 4-in-1.nes */
	{ 0x12DDA422,  176,       0, DEFAULT, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* (KT-8404) Super Game 4-in-1.nes */
	{ 0xAB09C88B,  176,       0, DEFAULT, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* (KT-8405) 4-in-1.nes */
	{ 0xA2E2031C,  176,       0, DEFAULT, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* (KT-8406) 4-in-1.nes */
	{ 0x6063F4D5,  176,       0, DEFAULT, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* (KT-A) Super Game.nes */
	{ 0xD2F5F51D,  176,       0, DEFAULT, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* (KY-1008) 10-in-1.nes */
	{ 0xBF31508D,  176,       0, DEFAULT, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* (KY-1102) Super 11-in-1.nes */
	{ 0x2B882971,  176,       0, DEFAULT, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* (KY-1501) 999999-in-1.nes */
	{ 0x981AE6BB,  176,       0, DEFAULT, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* (KY-1901) 19-in-1.nes */
	{ 0x5D2129AC,  176,       0, DEFAULT, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* (KY-2001) 20-in-1.nes */
	{ 0x9AD9A8E9,  176,       0, DEFAULT, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* (KY-6006) 6-, 4-in-1.nes */
	{ 0x521AF87B,  176,       0, DEFAULT, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* (KY-6009) 6-in-1 (FK23C).nes */
	{ 0xAA05C592,  176,       0, DEFAULT, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* (KY-6011) Super 6-in-1.nes */
	{ 0x205CF073,  176,       0, DEFAULT, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* (KY-8002) 8-in-1 Supergame.nes */
	{ 0xD408F0BC,  176,       0, DEFAULT, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* (KY-9005) 9-in-1 (FK23C).nes */
	{ 0x205D476E,  176,       0, DEFAULT, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* (KY-9006) 9-in-1 Super Game.nes */
	{ 0x0B2CF73F,  176,       0, DEFAULT, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* (MK-035) 3-in-1.nes */
	{ 0x2D1C667A,  176,       0, DEFAULT, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* (MK-036) 3-in-1.nes */
	{ 0x25D7CB92,  176,       0, DEFAULT, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* (MK-038) 4-in-1.nes */
	{ 0x8EFF9139,  176,       0, DEFAULT, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* (MK-042) Super Game 3-in-1.nes */
	{ 0xB55103AD,  176,       0, DEFAULT, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* (MK-042) Super Game 3-in-1 [game name correction] */
	{ 0xE83E5726,  176,       0, DEFAULT, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* (MK-064) Super Game 4-in-1.nes */
	{ 0xBEEB0B07,  176,       0, DEFAULT, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* (MK-064) Super Game 4-in-1 [game name correction].nes */
	{ 0xBBFBECBC,  176,       0, DEFAULT, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* (SB-04) 4-in-1.nes */
	{ 0x2A1D5933,  176,       0, DEFAULT, 0, DEFAULT,    0x07, DEFAULT, NOEXTRA }, /* (SC-03) Super 24-in-1.nes */
	{ 0x07564AC6,  176,       0, DEFAULT, 0, DEFAULT,    0x07, DEFAULT, NOEXTRA }, /* (SD-01) 絡克家族大集合 - Rockman 16-in-1.nes */
	{ 0xD7D2123C,  176,       0, DEFAULT, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* (VT-087) 4-in-1.nes */
	{ 0x8CB6D32C,  176,       0, DEFAULT, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* (VT-089) 4-in-1.nes */
	{ 0x23994975,  176,       0, DEFAULT, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* (VT-089) 4-in-1 [Russian].nes */
	{ 0x93196E95,  176,       0, DEFAULT, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* (VT-208) 4-in-1.nes */
	{ 0xECFE604A,  176,       0, DEFAULT, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* (YH-2001) 2-in-1 Mortal-Kombat.nes */
	{ 0xA232AA4E,  176,       0, DEFAULT, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* (YH-3128) Super Game 3-in-1.nes */
	{ 0xF565C023,  176,       0, DEFAULT, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* (YH-3132) Super Game 3-in-1.nes */
	{ 0x01131224,  176,       0, DEFAULT, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* (YH-3133) Super Game 3-in-1.nes */
	{ 0x7A76F2B9,  176,       0, DEFAULT, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* (YH-3135) Super Game 3-in-1.nes */
	{ 0xFCACB02A,  176,       0, DEFAULT, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* (YH-346) 3-in-1.nes */
	{ 0x5ADDD942,  176,       0, DEFAULT, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* (YH-360) Super Game 3-in-1.nes */
	{ 0x940933DC,  176,       0, DEFAULT, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* (YH-363) 3-in-1.nes */
	{ 0xC06C9AF7,  176,       0, DEFAULT, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* (YH-370) Super Game 3-in-1.nes */
	{ 0x728CA4AF,  176,       0, DEFAULT, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* (YH-401) 4-in-1.nes */
	{ 0xF019BFEF,  176,       0, DEFAULT, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* (YH-415) Super Game 4-in-1 */
	{ 0x4D25A3A6,  176,       0, DEFAULT, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* (YH-415) Super Game 4-in-1 [game name correction] */
	{ 0x5F96184B,  176,       0, DEFAULT, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* (YH-4102) 4-in-1.nes [bad CHR] */
	{ 0xC4D1D2F8,  176,       0, DEFAULT, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* (YH-4102) 4-in-1.nes */
	{ 0x6D580074,  176,       0, DEFAULT, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* (YH-4118) 4-in-1.nes */
	{ 0xFA5B1D26,  176,       0, DEFAULT, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* (YH-4122) 4-in-1 [b].nes */
	{ 0x4410BD8D,  176,       0, DEFAULT, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* (YH-4126) Super Game 4-in-1.nes */
	{ 0xC3166E11,  176,       0, DEFAULT, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* (YH-4146) 4-in-1.nes */
	{ 0xBE05120A,  176,       0, DEFAULT, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* (YH-417) Super Game 4-in-1.nes */
	{ 0x43BDB0FF,  176,       0, DEFAULT, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* (YH-4219) Super Game 4-in-1.nes */
	{ 0xF371BCF2,  176,       0, DEFAULT, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* (YH-4228) Super Game 4-in-1.nes */
	{ 0xFA1CB05C,  176,       0, DEFAULT, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* (YH-4237) 4-in-1.nes */
	{ 0x44F46BBC,  176,       0, DEFAULT, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* (YH-4239) New 4-in-1 Supergame.nes */
	{ 0x0A486DD7,  176,       0, DEFAULT, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* (YH-4242) Super Game 4-in-1.nes */
	{ 0x634036DB,  176,       0, DEFAULT, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* (YH-4246) Super Game 4-in-1.nes */
	{ 0xD7A8AFCA,  176,       0, DEFAULT, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* (YH-4247) Super Game 4-in-1.nes */
	{ 0x6AB68F4F,  176,       0, DEFAULT, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* (YH-4248) Super Game 4-in-1.nes */
	{ 0x1EF30CC8,  176,       0, DEFAULT, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* (YH-4253) 4-in-1.nes */
	{ 0x4D18054C,  176,       0, DEFAULT, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* (YH-437) Super Game 4-in-1.nes */
	{ 0x18DD93BC,  176,       0, DEFAULT, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* (YH-437) Super Game 4-in-1 [game name correction].nes */
	{ 0x63D43F22,  176,       0, DEFAULT, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* (YH-451) 4-in-1.nes */
	{ 0xBF31508D,  176,       0, DEFAULT, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* (YH-469) Super 4-in-1.nes */
	{ 0x981AE6BB,  176,       0, DEFAULT, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* (YH-480) 4-in-1.nes */
	{ 0x5D2129AC,  176,       0, DEFAULT, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* (YH-481) 4-in-1.nes */
	{ 0x5D061E04,  176,       0, DEFAULT, 0,    0x07, DEFAULT, DEFAULT, NOEXTRA }, /* (YH-602) 6-in-1.nes */
	{ 0xD2A4A9C6,  176,       0, DEFAULT, 0,    0x07, DEFAULT, DEFAULT, NOEXTRA }, /* (YH-602) Super Game 6-in-1 [game name correction, font change].nes */
	{ 0x35974F2C,  176,       0, DEFAULT, 0,    0x07, DEFAULT, DEFAULT, NOEXTRA }, /* (YH-602) Super Game 6-in-1 [font change].nes */
	{ 0x5B17FB27,  176,       0, DEFAULT, 0,    0x07, DEFAULT, DEFAULT, NOEXTRA }, /* (YH-801) Super Game 8-in-1.nes */
	{ 0x0881169E,  176,       0, DEFAULT, 0,    0x07, DEFAULT, DEFAULT, NOEXTRA }, /* (YH-801) Super Game 8-in-1 [font change].nes */
	{ 0xCD55A865,  176,       0, DEFAULT, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* (YH-8023) 4-in-1.nes */
	{ 0x92B52357,  176,       0, DEFAULT, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* (YH-8028) 4-in-1.nes */
	{ 0xDE94B7FD,  176,       0, DEFAULT, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* (YH-8029) 4-in-1.nes */	
	{ 0xD56F27AE,  176,       0, DEFAULT, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* (YH-8046) 4-in-1.nes */
	{ 0x0E8766DC,  176,       0, DEFAULT, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* (YH-8049) 4-in-1.nes */
	{ 0x05E43745,  176,       0, DEFAULT, 0,    0x07, DEFAULT, DEFAULT, NOEXTRA }, /* (YH-904) Super Game 9-in-1.nes */
	{ 0x31CADEF3,  176,       0, DEFAULT, 0,    0x07, DEFAULT, DEFAULT, NOEXTRA }, /* (YH-904) Super Game 9-in-1 [font change].nes */
	{ 0x04398A9F,  176,       0, DEFAULT, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* (卡聖 NT-945) 2-in-1.nes */
	{ 0x9BDF2424,  176,       0, DEFAULT, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* 12-in-1 Console TV Game Cartridge.nes */
	{ 0x629FBEEC,  176,       0, DEFAULT, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* 125-in-1.nes */
	{ 0x6EDED153,  176,       0, DEFAULT, 0,    0x07, DEFAULT, DEFAULT, NOEXTRA }, /* 16-in-1 Battle Hymn of the Republic.nes */
	{ 0x68EDFDEA,  176,       0, DEFAULT, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* 3-in-1 (Mortal Kombat MK5).nes */
	{ 0x8E994BCD,  176,       0, DEFAULT, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* 3-in-1 (The Lion King 5).nes */
	{ 0x104D84DF,  176,       0, DEFAULT, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* 3in1 (ES-Q800C) (Unl).nes */
	{ 0xDC904F4C,  176,       0, DEFAULT, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* 3in1 (ES-Q800C)(FSS).nes */
	{ 0xCF7C6AE0,  176,       0, DEFAULT, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* 3in1 (ES-Q800C)(TRS).nes */
	{ 0x4E5F123A,  176,       0, DEFAULT, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* 4-in-1 (Digital Adventure).nes */
	{ 0x7DB2D1F5,  176,       0, DEFAULT, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* 6-in-1 (Spiderman 2, Aladdin).nes */
	{ 0x75DFF1A4,  176,       0, DEFAULT, 0, DEFAULT,    0x07, DEFAULT, NOEXTRA }, /* 6-in-1 (Super Rockman).nes */
	{ 0xACCC9036,  176,       0, DEFAULT, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* 9-in-1 (Pokemon Yellow).nes */
	{ 0x674D6CDE,  176,       0, DEFAULT, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* Intellivision X2 15-in-1 [missing CHR data].nes */
	{ 0x0F907E7F,  176,       0, DEFAULT, 0, DEFAULT,    0x07, DEFAULT, NOEXTRA }, /* Rockman 6-in-1 (rev0).nes */
	{ 0xE6D869ED,  176,       0, DEFAULT, 0, DEFAULT,    0x07, DEFAULT, NOEXTRA }, /* Rockman 6-in-1 (rev0)[missing CHR data].nes */
	{ 0x88E0C48C,  176,       0, DEFAULT, 0, DEFAULT,    0x07, DEFAULT, NOEXTRA }, /* Rockman 6-in-1 (rev1).nes */
	{ 0xEECF01BF,  176,       0, DEFAULT, 0, DEFAULT,    0x07, DEFAULT, NOEXTRA }, /* Rockman 6-in-1 (rev3).nes */
	{ 0xC043A8DF,  176,       0, DEFAULT, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* 小博士教育软件系列꞉ 数学 小狀元.nes */
	{ 0x2CF5DB05,  176,       0, DEFAULT, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* 小博士教育软件系列꞉ 智力 小狀元.nes */
	{ 0x02C41438,  176,       0, DEFAULT, 1,    0x70,    0x07, DEFAULT, NOEXTRA }, /* 星河战士.nes */
	{ 0xA39C9A6B,  176,       0, DEFAULT, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* (KY-1203) Super Game 12-in-1/(YH-467) Super Game 4-in-1 */
	{ 0x0F05C0E6,  176,       0, DEFAULT, 0,    0x07, DEFAULT, DEFAULT, NOEXTRA }, /* (YH-701) Super Game 4-in-1 */
	{ 0x80F1E11E,  176,       0, DEFAULT, 0,    0x07, DEFAULT, DEFAULT, NOEXTRA }, /* (YH-701) Super Game 4-in-1 [game name correction, font change] */
	{ 0xC5C30EFA,  176,       0, DEFAULT, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* (YH-8011) Super Game 4-in-1 */
	{ 0x8858D3F7,  176,       0, DEFAULT, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* (YH-8011) Super Game 4-in-1 [game name correction] */
	{ 0x18BEB276,  176,       0, DEFAULT, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* (YH-8024) Super Game 4-in-1 */
	{ 0xE3A0B9E5,  176,       0, DEFAULT, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* (YH-8027) Super Game 4-in-1 */
	{ 0x5A0E3E69,  176,       0, DEFAULT, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* (YH-8033) Super Game 4-in-1 */
	{ 0x21D4484A,  176,       0, DEFAULT, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* (YH-8042) Super Game 4-in-1 */
	{ 0xF011AFD6,    4,       0, DEFAULT, 0,    0x70,    0x07, DEFAULT, NOEXTRA }, /* Rockman 4- Minus Infinity [Infinite Life]. Basically oversize MMC3 that fails to enable WRAM before accessing it. */
	{ 0x576D9589,  176,       1, DEFAULT, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* (BS-0210A PCB) Super Mario 4-in-1.nes */
	{ 0x01B3EDD2,  176,       1, DEFAULT, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* (BS-6002) Super Game 16-in-1.nes */
	{ 0x291F5318,  176,       1, DEFAULT, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* (BS-6008) 210-in-1.nes */
	{ 0xB12CCB95,  176,       1, DEFAULT, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* (BS-6028) 180-in-1.nes */
	{ 0xD5281CF3,  176,       1, DEFAULT, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* (BS-8002) New 4-in-1.nes */
	{ 0x1F6AC22E,  176,       1, DEFAULT, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* (BS-8005) Super Game 3-in-1.nes */
	{ 0xB48D6F26,  176,       1, DEFAULT, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* (BS-8008) Super Game 4-in-1.nes */
	{ 0xBDDA85CE,  176,       1, DEFAULT, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* (BS-8013) Super Game 4-in-1.nes */
	{ 0x85C2CC88,  176,       1, DEFAULT, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* (BS-8029) Super Game 4-in-1.nes */
	{ 0x4F2CCD03,  176,       1, DEFAULT, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* (BS-8029) Super Game 4-in-1 [overdump] */
	{ 0x40182FF6,  176,       1, DEFAULT, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* (BS-8045) Super Game 4-in-1.nes */
	{ 0xF4CE452C,  176,       1, DEFAULT, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* (BS-8062) Super Game 4-in-1.nes */
	{ 0x409601A5,  176,       1, DEFAULT, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* (BS-8088) 4-in-1.nes */
	{ 0xC8AB31FD,  176,       1, DEFAULT, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* (BS-8103) Super Game 4-in-1.nes */
	{ 0x87308F9D,  176,       1, DEFAULT, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* (BS-8105) Super Game 4-in-1.nes */
	{ 0x95ACA7A7,  176,       1, DEFAULT, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* (FK-012) 80-in-1.nes */
	{ 0xE650EC91,  176,       1, DEFAULT, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* (FK-014) 128-in-1.nes */
	{ 0x7642F6B6,  176,       1, DEFAULT, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* (FK-021) 180-in-1.nes */
	{ 0x24762CE8,  176,       1, DEFAULT, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* (FK-022) 178-in-1.nes */
	{ 0x761CF0C0,  176,       1, DEFAULT, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* (FK-026) 210-in-1.nes */
	{ 0x165102DB,  176,       1, DEFAULT, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* (FK-033) 52-in-1.nes */
	{ 0x6614D4C0,  176,       1, DEFAULT, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* (FK-037) Super Game 16-in-1.nes */
	{ 0xB3277B6C,  176,       1, DEFAULT, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* (FK-3004) 6-in-1.nes */
	{ 0x3907578B,  176,       1, DEFAULT, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* (FK-8008) 4-in-1.nes */
	{ 0xA391549D,  176,       1, DEFAULT, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* (FK-8021) 4-in-1.nes */
	{ 0x36C27AE8,  176,       1, DEFAULT, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* (FK-8021) 4-in-1.nes [PRG banks missing] */
	{ 0x06D13D9E,  176,       1, DEFAULT, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* (FK-8026) 4-in-1.nes */
	{ 0x2EBD5FD6,  176,       1, DEFAULT, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* (FK-8033) 4-in-1.nes */
	{ 0x23E4906A,  176,       1, DEFAULT, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* (FK-8043) 4-in-1.nes */
	{ 0x10155A92,  176,       1, DEFAULT, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* (FK-8045) 4-in-1.nes */
	{ 0xF66944EE,  176,       1, DEFAULT, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* (FK-8050) 4-in-1.nes */
	{ 0x8BAEEDC0,  176,       1, DEFAULT, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* (FK-8052) 4-in-1.nes */
	{ 0x39307391,  176,       1, DEFAULT, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* (FK-8056) 4-in-1.nes */
	{ 0x72CEAB1E,  176,       1, DEFAULT, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* (FK-8078) 4-in-1.nes */
	{ 0x05A1F101,  176,       1, DEFAULT, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* (FK-8078) 4-in-1 [bad CHR].nes */
	{ 0x07D3F6CB,  176,       1, DEFAULT, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* (FK-8079) 4-in-1.nes */
	{ 0x4E34CC0A,  176,       1, DEFAULT, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* (FK-8161) Super Game 12-in-1.nes */
	{ 0x20379331,  176,       1, DEFAULT, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* (FK-8244) 7-in-1.nes */
	{ 0x622E9E35,  176,       1, DEFAULT, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* (FK-xxx) 126-in-1.nes */
	{ 0x666E736D,  176,       1, DEFAULT, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* (FK-xxx) Super 15-in-1.nes */
	{ 0x19195C7F,  176,       1, DEFAULT, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* (FK-xxx) Super 35-in-1.nes */
	{ 0x6F775C1A,  176,       1, DEFAULT, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* (FK-xxx) Super 8-in-1.nes */
	{ 0x17D43AF9,  176,       1, DEFAULT, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* (K-5003) 5-in-1.nes */
	{ 0xFE384E4B,  176,       1, DEFAULT, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* (KB-0306N PCB) Super Game 4-in-1 #1.nes */
	{ 0x425F5325,  176,       1, DEFAULT, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* (KB-0306N PCB) Super Game 4-in-1 #2.nes */
	{ 0xD206A6DF,  176,       1, DEFAULT, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* (KB-4004) Super 4-in-1.nes */
	{ 0xF350556E,  176,       1, DEFAULT, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* (KB-4009) Super 4-in-1.nes */
	{ 0x14BD2D1C,  176,       1, DEFAULT, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* (KB-4016) Super 4-in-1 [fix Legend of Kage].nes */
	{ 0xBB9258BE,  176,       1, DEFAULT, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* (KB-4016) Super 4-in-1.nes */
	{ 0x6343E6A6,  176,       1, DEFAULT, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* (KD-1512 PCB) 128-in-1.nes */
	{ 0x91A8CD7E,  176,       1, DEFAULT, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* (KD-1512 PCB) Super 20-in-1.nes */
	{ 0xE31220BE,  176,       1, DEFAULT, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* (KD-6020) Super Game 131-in-1.nes */
	{ 0xB623C3D0,  176,       1, DEFAULT, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* (KD-6024) Super Game 168-in-1.nes */
	{ 0x30FFB076,  176,       1, DEFAULT, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* (KD-6033) 7-in-1.nes */
	{ 0xF0C581B3,  176,       1, DEFAULT, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* (KG-6009) Super Game 58-in-1.nes */
	{ 0x576EC760,  176,       1, DEFAULT, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* (KT-3445) Super 4-in-1.nes */
	{ 0xBF015F20,  176,       1, DEFAULT, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* (KT-3445A-1) Super 4-in-1.nes */
	{ 0x0163CA53,  176,       1, DEFAULT, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* (KT-3445A-B) 4-in-1.nes */
	{ 0x3DF39CE4,  176,       1, DEFAULT, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* (KT-4403) Super 3-in-1.nes */
	{ 0xB8FED144,  176,       1, DEFAULT, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* (KT-8017) Super Game 4-in-1.nes */
	{ 0x10E7B6ED,  176,       1, DEFAULT, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* (KT-8040) Super Game 4-in-1.nes */
	{ 0x38E63310,  176,       1, DEFAULT, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* (KT-8109) Super Game 4-in-1.nes */
	{ 0xC6D97331,  176,       1, DEFAULT, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* (BS-0210A PCB) 4-in-1.nes */
	{ 0x9367D1F4,  176,       1, DEFAULT, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* (BS-0210A PCB) 4-in-1 [Russian].nes */
	{ 0xE6617BF1,  176,       1, DEFAULT, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* (SJ-0027) 180-in-1.nes */
	{ 0x9206B787,  176,       1, DEFAULT, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* (YE-C011) 60-in-1.nes */
	{ 0x32CDAD83,  176,       1, DEFAULT, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* (晶太 JY-222) 1998 97格鬥天王 激鬥篇 6-in-1.nes */
	{ 0x3C894AD1,  176,       0, DEFAULT, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* (晶太 JY-224) 1998 97格鬥天王 激鬥篇 7-in-1.nes */
	{ 0x78CE996D,  176,       1, DEFAULT, 0,    0x07, DEFAULT, DEFAULT, NOEXTRA }, /* (晶太 JY-225) 1998 97格鬥天王 激戰篇 6-in-1.nes */
	{ 0xB5D25A20,  176,       1, DEFAULT, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* (BS-0306M PCB) Super 4-in-1.nes */
	{ 0x8C48BDBA,  176,       1, DEFAULT, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* (BS-0306M PCB) Super 4-in-1 #2.nes */
	{ 0xC51FA465,  176,       1, DEFAULT, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* (KD-6032) 180-in-1.nes */
	{ 0xA22DE99D,  176,       1, DEFAULT, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* (KD-6026) Super Game 210-in-1 */
	{ 0xE8BD5AC3,  176,       1, DEFAULT, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* (BS-6017) Super Game 28-in-1 */
	{ 0x37478F0C,  176,       1, DEFAULT, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* (BS-8009) Super 4-in-1 */
	{ 0x98C59170,  176,       1, DEFAULT, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* (BS-8014) Super 4-in-1 */
	{ 0x7CA43C89,  176,       1, DEFAULT, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* (BS-8117) Super Game 4-in-1 */
	{ 0x678DE5AA,  176,       2, DEFAULT, 0, DEFAULT,    0x0C,   DENDY, NOEXTRA }, /* (外星) 120-in-1.nes */
	{ 0x6C979BAC,  176,       2, DEFAULT, 0, DEFAULT,    0x0B,   DENDY, NOEXTRA }, /* (奔升) 10-in-1 Omake Game.nes */
	{ 0xE79F157E,  176,       2, DEFAULT, 0, DEFAULT,    0x0B,   DENDY, NOEXTRA }, /* (福州 Coolboy) 245-in-1 Real Game.nes */
	{ 0x3AE35EC1,  176,       2, DEFAULT, 0, DEFAULT,    0x0B,   DENDY, NOEXTRA }, /* (福州 Coolboy) 400-in-1 Real Game.nes */
	{ 0xD14617D7,  176,       2, DEFAULT, 0, DEFAULT,    0x0B,   DENDY, NOEXTRA }, /* (福州 Coolboy) 合金装备 150-in-1 Real Game.nes */
	{ 0x37290B20,  176,       2, DEFAULT, 1,    0x90,    0x07,   DENDY, NOEXTRA }, /* Age of Empires - 帝国时代 (970493) [protection removed].nes */
	{ 0x5EE2EF97,  176,       2, DEFAULT, 1,    0x90,    0x07,   DENDY, NOEXTRA }, /* Age of Empires - 帝国时代 (970493).nes */
	{ 0x5077CAC1,  176,       2, DEFAULT, 1,    0x90,    0x07,   DENDY, NOEXTRA }, /* EverQuest - 八宝奇珠.nes */
	{ 0xF354D847,  176,       2, DEFAULT, 1,    0x90,    0x07,   DENDY, NOEXTRA }, /* Grandia - 格蘭蒂亞傳說 (2006SR04308).nes */
	{ 0xD51DD22C,  176,       2, DEFAULT, 1,    0x90,    0x07,   DENDY, NOEXTRA }, /* SD 高达外传 - 骑士高达故事 3꞉ 传说之骑士团.nes */
	{ 0xF29C8186,  176,       2, DEFAULT, 1,    0x90,    0x07,   DENDY, NOEXTRA }, /* Shanghai Tycoon - 上海大亨 (960313).nes */
	{ 0xF2398802,  176,       2, DEFAULT, 1,    0x90,    0x07,   DENDY, NOEXTRA }, /* 七龙珠 2꞉ 激战弗利萨!!.nes */
	{ 0x53B62838,  176,       2, DEFAULT, 1,    0x90,    0x07,   DENDY, NOEXTRA }, /* 七龙珠 2꞉ 电光石火.nes */
	{ 0xED481B7C,  176,       2, DEFAULT, 1,    0x90,    0x07,   DENDY, NOEXTRA }, /* 七龙珠 Z 外传꞉ 赛亚人灭绝计划 (rev1).nes */
	{ 0xC768098B,  176,       2, DEFAULT, 1,    0x90,    0x07,   DENDY, NOEXTRA }, /* 三侠五义꞉ 御猫傳奇 (2006SR04307).nes */
	{ 0xB511C04B,  176,       2, DEFAULT, 1,    0x90,    0x07,   DENDY, NOEXTRA }, /* 三侠五义꞉ 御猫傳奇 (2006SR04307)[protection removed].nes */
	{ 0x44C20420,  176,       2, DEFAULT, 1,    0x90,    0x07,   DENDY, NOEXTRA }, /* 三国志 II - 中文加强版.nes */
	{ 0x8FFC1864,  176,       2, DEFAULT, 1,    0x90,    0x07,   DENDY, NOEXTRA }, /* 三国志 II꞉ 覇王の大陸.nes */
	{ 0x8F6AB5AC,  176,       2, DEFAULT, 1,    0x90,    0x07,   DENDY, NOEXTRA }, /* 三国忠烈传 - The Story of Three Kingdoms (FS005).nes */
	{ 0x94782FBD,  176,       2, DEFAULT, 1,    0x90,    0x07,   DENDY, NOEXTRA }, /* 三國志꞉ 雄霸天下 (980100026).nes */
	{ 0x99051CB5,  176,       2, DEFAULT, 1,    0x90,    0x07,   DENDY, NOEXTRA }, /* 三國志꞉ 雄霸天下 (980337).nes */
	{ 0x3A1CFE21,  176,       2, DEFAULT, 1,    0x90,    0x07,   DENDY, NOEXTRA }, /* 上古神殿.nes */
	{ 0xD6EA31C0,  176,       2, DEFAULT, 1,    0x90, DEFAULT,   DENDY, NOEXTRA }, /* 东周列国志 (2006SR04301).nes */
	{ 0x03346083,  176,       2, DEFAULT, 1,    0x90, DEFAULT,   DENDY, NOEXTRA }, /* 东周列国志 (2006SR04301)[protection removed].nes */
	{ 0x33443508,  176,       2, DEFAULT, 1,    0x90, DEFAULT,   DENDY, NOEXTRA }, /* 争霸世纪 (960268).nes */
	{ 0x7696573A,  176,       2, DEFAULT, 1,    0x90,    0x07,   DENDY, NOEXTRA }, /* 亚特鲁战记.nes */
	{ 0xFE383376,  176,       2, DEFAULT, 1,    0x90,    0x07,   DENDY, NOEXTRA }, /* 仙剑神曲 - Space General [protection removed].nes */
	{ 0x97F3D7C1,  176,       2, DEFAULT, 1,    0x90,    0x07,   DENDY, NOEXTRA }, /* 仙剑神曲 - Space General.nes */
	{ 0x8264EA52,  176,       2, DEFAULT, 1,    0x90, DEFAULT,   DENDY, NOEXTRA }, /* 侠客情꞉ 荆轲刺秦王 (FS005).nes */
	{ 0x027FD794,  176,       2, DEFAULT, 1,    0x90, DEFAULT,   DENDY, NOEXTRA }, /* 剑舞者 - Sword Dancer.nes */
	{ 0x095D8678,  176,       2, DEFAULT, 1,    0x90,    0x07,   DENDY, NOEXTRA }, /* 双月传.nes */
	{ 0xD5F7AAEF,  176,       2, DEFAULT, 1,    0x90,    0x07,   DENDY, NOEXTRA }, /* 神风剑.nes */
	{ 0x377FDB36,  176,       2, DEFAULT, 1,    0x90,    0x07,   DENDY, NOEXTRA }, /* 口袋精靈꞉ 金.nes */
	{ 0x85DD49B6,  176,       2, DEFAULT, 1,    0x90,    0x07,   DENDY, NOEXTRA }, /* 口袋精靈꞉ 金 [protection removed].nes */
	{ 0x3F6124C3,  176,       2, DEFAULT, 1,    0x90,    0x07,   DENDY, NOEXTRA }, /* 哆啦A梦 - 超时空历险 [a].nes */
	{ 0x69A3CA5C,  176,       2, DEFAULT, 1,    0x90,    0x07,   DENDY, NOEXTRA }, /* 哆啦A梦 - 超时空历险.nes */
	{ 0x548D72FF,  176,       2, DEFAULT, 1,    0x90,    0x07,   DENDY, NOEXTRA }, /* 哥伦布 冒险记 - 黄金中文版.nes */
	{ 0xEEBEE0C8,  176,       2, DEFAULT, 1,    0x90,    0x07,   DENDY, NOEXTRA }, /* 哥伦布传 - 黄金中文版 (FS006).nes */
	{ 0x3C9DF646,  176,       2, DEFAULT, 1,    0x90,    0x07,   DENDY, NOEXTRA }, /* 圣斗士.nes */
	{ 0x52A5F554,  176,       2, DEFAULT, 1,    0x90, DEFAULT,   DENDY, NOEXTRA }, /* 基督山恩仇记 - Le Comte de Monte Cristo (970260).nes */
	{ 0x35F8BD75,  176,       2, DEFAULT, 1,    0x90, DEFAULT,   DENDY, NOEXTRA }, /* 基督山恩仇记 - Le Comte de Monte Cristo (970260)[protection removed].nes */
	{ 0xEE49F509,  176,       2, DEFAULT, 1,    0x90,    0x07,   DENDY, NOEXTRA }, /* 大盗伍佑卫门之天下宝藏.nes */
	{ 0xC04D330D,  176,       2, DEFAULT, 1,    0x90, DEFAULT,   DENDY, NOEXTRA }, /* 夺宝小英豪꞉ 光明與黑暗傳説 (960270).nes */
	{ 0x848F2D69,  176,       2, DEFAULT, 1,    0x90,    0x07,   DENDY, NOEXTRA }, /* 女神的救赎.nes */
	{ 0x2C3D4EF0,  176,       2, DEFAULT, 1,    0x90,    0x07,   DENDY, NOEXTRA }, /* 宠物꞉ 翡翠 (2004SR05368).nes */
	{ 0x09FC02C7,  176,       2, DEFAULT, 1,    0x90,    0x07,   DENDY, NOEXTRA }, /* 宠物꞉ 翡翠 (2004SR05368)[protection removed].nes */
	{ 0x913B95F7,  176,       2, DEFAULT, 1,    0x90,    0x07,   DENDY, NOEXTRA }, /* 富甲三国 [protection removed].nes */
	{ 0x0F73D488,  176,       2, DEFAULT, 1,    0x90,    0x07,   DENDY, NOEXTRA }, /* 富甲三国.nes */
	{ 0x8947AB85,  176,       2, DEFAULT, 1,    0x90,    0x07,   DENDY, NOEXTRA }, /* 少年游侠꞉ 光明之神 - Young Chivalry (FS005).nes */
	{ 0xEE66C5E8,  176,       2, DEFAULT, 1,    0x90, DEFAULT,   DENDY, NOEXTRA }, /* 帝国风暴 - Napoleon's War (980340) [protection removed].nes */
	{ 0x351DD533,  176,       2, DEFAULT, 1,    0x90, DEFAULT,   DENDY, NOEXTRA }, /* 帝国风暴 - Napoleon's War (980340).nes */
	{ 0xEABBB630,  176,       2, DEFAULT, 1,    0x90,    0x07,   DENDY, NOEXTRA }, /* 怪物制造者 1.nes */
	{ 0xF1D40F5B,  176,       2, DEFAULT, 1,    0x90,    0x07,   DENDY, NOEXTRA }, /* 怪物制造者 2.nes */
	{ 0x2661109F,  176,       2, DEFAULT, 1,    0x90, DEFAULT,   DENDY, NOEXTRA }, /* 戰國無雙 [protection removed].nes */
	{ 0xCC6E548C,  176,       2, DEFAULT, 1,    0x90, DEFAULT,   DENDY, NOEXTRA }, /* 戰國無雙.nes */
	{ 0xBA29435A,  176,       2, DEFAULT, 1,    0x90, DEFAULT,   DENDY, NOEXTRA }, /* 戰國群雄傳 (970498).nes */
	{ 0x5A88B5B0,  176,       2, DEFAULT, 1,    0x90,    0x07,   DENDY, NOEXTRA }, /* 数码暴龙 4꞉ 水晶版 (2004SR01259).nes */
	{ 0xA46353D1,  176,       2, DEFAULT, 1,    0x90, DEFAULT,   DENDY, NOEXTRA }, /* 杨家将 - Yang's Troops (980186).nes */
	{ 0xC155128F,  176,       2, DEFAULT, 1,    0x90, DEFAULT,   DENDY, NOEXTRA }, /* 梁山英雄传 [protection removed].nes */
	{ 0xAD223177,  176,       2, DEFAULT, 1,    0x90, DEFAULT,   DENDY, NOEXTRA }, /* 梁山英雄传.nes */
	{ 0xBF6E95F5,  176,       2, DEFAULT, 1,    0x90,    0x07,   DENDY, NOEXTRA }, /* 梦境之王 - Dream Master.nes */
	{ 0x416C07A1,  176,       2, DEFAULT, 1,    0x90,    0x07,   DENDY, NOEXTRA }, /* 梦幻之星 IV.nes */
	{ 0x852CE16B,  176,       2, DEFAULT, 1,    0x90, DEFAULT,   DENDY, NOEXTRA }, /* 楚漢爭霸 - The War Between Chu & Han (980005).nes */
	{ 0xE0DD8D77,  176,       2, DEFAULT, 1,    0x90, DEFAULT,   DENDY, NOEXTRA }, /* 楚留香꞉ 香帥傳奇之血海飘零 (FS005).nes */
	{ 0x2E4F3051,  176,       2, DEFAULT, 1,    0x90, DEFAULT,   DENDY, NOEXTRA }, /* 楚留香꞉ 香帥傳奇之血海飘零 (FS005)[protection removed].nes */
	{ 0x34AC5AE9,  176,       2, DEFAULT, 1,    0x90, DEFAULT,   DENDY, NOEXTRA }, /* 水滸傳 - Marsh Outlaws (960415).nes */
	{ 0x1923A8C5,  176,       2, DEFAULT, 1,    0x90,    0x07,   DENDY, NOEXTRA }, /* 水滸神獸 [protection removed].nes */
	{ 0x6B4CAC80,  176,       2, DEFAULT, 1,    0x90,    0x07,   DENDY, NOEXTRA }, /* 水滸神獸.nes */
	{ 0xC9D968AF,  176,       2, DEFAULT, 1,    0x90,    0x07,   DENDY, NOEXTRA }, /* 混沌世界.nes */
	{ 0x7F3DBF1B,  176,       2, DEFAULT, 1,    0x90,    0x07,     PAL, NOEXTRA }, /* 混沌世界 [VirtuaNES PAL].nes */
	{ 0xB616885C,  176,       2, DEFAULT, 1,    0x90,    0x07,     PAL, NOEXTRA }, /* 混沌世界 [VirtuaNES PAL, SRAM bank switch removed].nes */
	{ 0xF031E7CD,  176,       2, DEFAULT, 1,    0x90,    0x07,   DENDY, NOEXTRA }, /* 激战弗利萨之孙悟饭.nes */
	{ 0xD871D3E6,  176,       2, DEFAULT, 1,    0x90,    0x07,   DENDY, NOEXTRA }, /* 激战弗利萨之孙悟饭 [bad CHR].nes */
	{ 0xC35E9AA8,  176,       2, DEFAULT, 1,    0x90,    0x07,   DENDY, NOEXTRA }, /* 激战弗利萨之孙悟饭 [Traditional Chinese].nes */
	{ 0xBFF7C60C,  176,       2, DEFAULT, 1,    0x90,    0x07,   DENDY, NOEXTRA }, /* 激斗圣战士.nes */
	{ 0x8CAAFF73,  176,       2, DEFAULT, 1,    0x90,    0x07,   DENDY, NOEXTRA }, /* 爆笑三國 - Jacks in Tri-Empire (970261)(FS005).nes */
	{ 0x97B82F53,  176,       2, DEFAULT, 1,    0x90,    0x07,   DENDY, NOEXTRA }, /* 爆笑三國 - Jacks in Tri-Empire (970261)(FS005)[protection removed].nes */
	{ 0x95569A86,  176,       2, DEFAULT, 1,    0x90,    0x07,   DENDY, NOEXTRA }, /* 甲A - China Soccer League for Division A (980333).nes */
	{ 0x977D22C3,  176,       2, DEFAULT, 1,    0x90,    0x07,   DENDY, NOEXTRA }, /* 破釜沉舟 [protection removed].nes */
	{ 0xA2DC64FF,  176,       2, DEFAULT, 1,    0x90,    0x07,   DENDY, NOEXTRA }, /* 破釜沉舟.nes */
	{ 0x356A16E9,  176,       2, DEFAULT, 1,    0x90, DEFAULT,   DENDY, NOEXTRA }, /* 絶代英雄 - Unrivalled Hero [protection removed].nes */
	{ 0xDA7D586D,  176,       2, DEFAULT, 1,    0x90, DEFAULT,   DENDY, NOEXTRA }, /* 絶代英雄 - Unrivalled Hero.nes */
	{ 0x50D5F94B,  176,       2, DEFAULT, 1,    0x90,    0x07,   DENDY, NOEXTRA }, /* 英烈群侠传 (980332).nes */
	{ 0xF5C16B14,  176,       2, DEFAULT, 1,    0x90,    0x07,   DENDY, NOEXTRA }, /* 荣耀之骑士团 [protection removed].nes */
	{ 0xFB2563D1,  176,       2, DEFAULT, 1,    0x90,    0x07,   DENDY, NOEXTRA }, /* 荣耀之骑士团.nes */
	{ 0xF9863ADF,  176,       2, DEFAULT, 1,    0x90,    0x07,   DENDY, NOEXTRA }, /* 西楚霸王 (980100461)(FS005).nes */
	{ 0x3FF36623,  176,       2, DEFAULT, 1,    0x90,    0x07,   DENDY, NOEXTRA }, /* 西楚霸王 (990031).nes */
	{ 0xF1D803F3,  176,       2, DEFAULT, 1,    0x90,    0x07,   DENDY, NOEXTRA }, /* 西楚霸王 (990031)[protection].nes */
	{ 0x49F22159,  176,       2, DEFAULT, 1,    0x90,    0x07,   DENDY, NOEXTRA }, /* 超级 大富翁 - Super Rich (FS005).nes */
	{ 0x2DDA2835,  176,       2, DEFAULT, 1,    0x90, DEFAULT,   DENDY, NOEXTRA }, /* 超级 大战略 [protection removed].nes */
	{ 0x7DCE29CB,  176,       2, DEFAULT, 1,    0x90, DEFAULT,   DENDY, NOEXTRA }, /* 超级 大战略.nes */
	{ 0x0DBFF515,  176,       2, DEFAULT, 1,    0x90,    0x07,   DENDY, NOEXTRA }, /* 银白色沙加.nes */
	{ 0x38EFFD3E,  176,       2, DEFAULT, 1,    0x90, DEFAULT,   DENDY, NOEXTRA }, /* 隋唐演义 (980185).nes */
	{ 0xCC56BCFE,  176,       2, DEFAULT, 1,    0x90,    0x07,   DENDY, NOEXTRA }, /* 風雲 - Traitor Legend (970100684)(FS005).nes */
	{ 0xFD883527,  176,       2, DEFAULT, 1,    0x90,    0x07,   DENDY, NOEXTRA }, /* 風雲 - Traitor Legend (980334).nes */
	{ 0xBC4AC7FE,  176,       2, DEFAULT, 1,    0x90,    0x07,   DENDY, NOEXTRA }, /* 高达骑士物语 2꞉ 光之骑士.nes */
	{ 0xB8FCD425,  176,       2, DEFAULT, 1,    0x90,    0x07,   DENDY, NOEXTRA }, /* 高达骑士物语.nes */
	{ 0xF17E49D5,  176,       2, DEFAULT, 1,    0x90,    0x07,   DENDY, NOEXTRA }, /* 鬼魅战记.nes */
	{ 0x4BCDB970,  176,       2, DEFAULT, 1,    0x90,    0x07,   DENDY, NOEXTRA }, /* 鬼魅战记.nes */
	{ 0x7D9C7206,  176,       2, DEFAULT, 1,    0x90, DEFAULT,   DENDY, NOEXTRA }, /* 魔域英雄传 - Hero on Devil Lands (FS005).nes */
	{ 0xB16D4268,  176,       2, DEFAULT, 1,    0x90, DEFAULT,   DENDY, NOEXTRA }, /* 魔法门之英雄无敌 (980584).nes */
	{ 0x71DAF1A6,  176,       2, DEFAULT, 1,    0x90, DEFAULT,   DENDY, NOEXTRA }, /* 魔法门之英雄无敌 (980584)[protection removed].nes */
	{ 0x3532A114,  176,       2, DEFAULT, 1,    0x90, DEFAULT,   DENDY, NOEXTRA }, /* 魔神法师 - Demogorgon Monk (FS005).nes */
	{ 0xE8EAFBC1,  176,       2, DEFAULT, 1,    0x90,    0x07,   DENDY, NOEXTRA }, /* 龙之谷 [protection removed].nes */
	{ 0x27DBC407,  176,       2, DEFAULT, 1,    0x90,    0x07,   DENDY, NOEXTRA }, /* 龙之谷.nes */
	{ 0xEBA09ADA,  176,       2, DEFAULT, 1,    0x90,    0x07,   DENDY, NOEXTRA }, /* 龙域天下.nes */
	{ 0x62DDE924,  176,       2, DEFAULT, 1,    0x90,    0x07,   DENDY, NOEXTRA }, /* 龙珠Z3꞉ 人造人列传.nes */
	{ 0x42161530,  176,       2, DEFAULT, 1,    0x90,    0x07,   DENDY, NOEXTRA }, /* 龙魂 [protection removed].nes */
	{ 0x8F8FC9A7,  176,       2, DEFAULT, 1,    0x90,    0x07,   DENDY, NOEXTRA }, /* 龙魂.nes */
	{ 0xB6984DAD,  176,       3, DEFAULT, 1,    0x07,    0x0B, DEFAULT, NOEXTRA }, /* Super Mario 160-in-1 Funny Time.nes */
	{ 0xAD82BBEA,  176,       4, DEFAULT, 1,    0x07,    0x07, DEFAULT, NOEXTRA }, /* GameStar Smart Genius Deluxe.nes */

	/* Nanjing games that are sometimes found erroneously set to mapper 163 even though they use other mappers */
	{ 0x3CD15707,  178,       0,    MI_V, 1,    0x70,    0x07,   DENDY, NOEXTRA }, /* [NJ027] Fang Shi Yu (C) */
	{ 0x2779BB41,  534,       0,    MI_V, 0, DEFAULT,    0x07,   DENDY, NOEXTRA }, /* [NJ064] Shu Du (Sudoku) (C) */
	{ 0x1DE558A1,  178,       0,    MI_V, 1,    0x70,    0x07,   DENDY, NOEXTRA }, /* [NJ085] Shan Shan De Hong Xing (C) */
	{ 0xF814EC57,  178,       0,    MI_V, 1,    0x70,    0x07,   DENDY, NOEXTRA }, /* [NJ090] Yong Zhe Chuan Shuo (C) */
	{ 0x53A1F436,  178,       0,    MI_V, 1,    0x70,    0x07,   DENDY, NOEXTRA }, /* [NJ091] Xian Jian Wen Qing (C) */

	/* Non-Mapper 176 files that are sometimes found erroneously set to mapper 176 */
	{ 0x60AC647F,  260,       0, DEFAULT, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* (YH-4222) Super Game 4-in-1.nes */

	{ 0x558c0dc3,  178, DEFAULT, DEFAULT, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* Super 2-in-1 (Soccer Game & Crazy Dance) (Unl) [!].nes */
	{ 0xc68363f6,  180, DEFAULT,    MI_H, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* Crazy Climber (Japan).nes */
	{ 0x0f05ff0a,  181, DEFAULT, DEFAULT, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* Seicross (Japan) (Rev 1).nes */
	{ 0x96ce586e,  189, DEFAULT, DFAULT8, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* Street Fighter II - The World Warrior (Unl) [!].nes */
	{ 0x0e76e4c1,  190, DEFAULT,    MI_V, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* Magic Kid Googoo (Korea) (Unl).nes */
	{ 0x555a555e,  191, DEFAULT, DEFAULT, 1, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* Sugoro Quest - Dice no Senshitachi (Ch).nes */
	{ 0x2cc381f6,  191, DEFAULT, DEFAULT, 1, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* Sugoro Quest - Dice no Senshitachi (Ch) [o1].nes */
	{ 0xa145fae6,  192, DEFAULT, DEFAULT, 1, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* Young Chivalry (Ch) [b3].nes */
	{ 0xa9115bc1,  192, DEFAULT, DEFAULT, 1, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* Young Chivalry (Ch) [!].nes */
	{ 0x4c7bbb0e,  192, DEFAULT, DEFAULT, 1, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* Young Chivalry (Ch) [b2].nes */
	{ 0x98c1cd4b,  192, DEFAULT, DEFAULT, 1, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* Ying Lie Qun Xia Zhuan (Asia) (Unl).nes */
	{ 0xee810d55,  192, DEFAULT, DEFAULT, 1, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* You Ling Xing Dong (Asia) (Unl).nes */
	{ 0x442f1a29,  192, DEFAULT, DEFAULT, 1, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* Young Chivalry (Ch) [b1].nes */
	{ 0x637134e8,  193, DEFAULT,    MI_H, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* Fighting Hero (Asia) (Ja) (Unl).nes */
	{ 0xa925226c,  194, DEFAULT, DEFAULT, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* Dai-2-Ji - Super Robot Taisen (Ch) [b1].nes */
	{ 0x33c5df92,  195, DEFAULT, DEFAULT, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* Captain Tsubasa Vol. II - Super Striker (Ch) [a1].nes */
	{ 0x1bc0be6c,  195, DEFAULT, DEFAULT, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* Captain Tsubasa Vol. II - Super Striker (Ch) [a3].nes */
	{ 0xd5224fde,  195, DEFAULT, DEFAULT, 1, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* God Slayer - Haruka Tenkuu no Sonata (Ch).nes */
	{ 0xfdec419f,  196, DEFAULT, DEFAULT, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* Street Fighter VI 16 Peoples (Unl) [!].nes */
	{ 0x700705f4,  198, DEFAULT, DEFAULT, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* ---- */
	{ 0x9a2cf02c,  198, DEFAULT, DEFAULT, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* ---- */
	{ 0xd8b401a7,  198, DEFAULT, DEFAULT, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* ---- */
	{ 0x28192599,  198, DEFAULT, DEFAULT, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* ---- */
	{ 0x19b9e732,  198, DEFAULT, DEFAULT, 1, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* Tenchi wo Kurau II - Shokatsu Koumei Den (J) (PRG0) [T-Chi][b6].nes */
	{ 0xdd431ba7,  198, DEFAULT, DEFAULT, 1, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* Tenchi wo kurau 2 (c) */
	{ 0x05658ded,  201,      15, DEFAULT, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* 21-in-1 (CF-043) (2006-V) (Unl) [p1].nes */
	{ 0x276237b3,  206, DEFAULT,    MI_H, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* Karnov (Japan) (Rev 1).nes */
	{ 0xa5e6baf9,  206, DEFAULT,    MI_V, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* Dragon Slayer IV - Drasle Family (Japan).nes */
	{ 0x4f2f1846,  206, DEFAULT,    MI_V, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* Famista '89 - Kaimaku Ban!! (Japan).nes */
	{ 0x22d6d5bd,  206, DEFAULT,    MI_V, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* Jikuu Yuuden - Debias (Japan).nes */
	{ 0x9d21fe96,  206, DEFAULT,    MI_V, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* Lupin Sansei - Pandora no Isan (Japan).nes */
	{ 0xae321339,  206, DEFAULT,    MI_V, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* Pro Yakyuu - Family Stadium '88 (Japan).nes */
	{ 0x96dfc776,  206, DEFAULT,    MI_V, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* R.B.I. Baseball 2 (USA) (Unl).nes */
	{ 0xfd63e7ac,  206, DEFAULT,    MI_V, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* R.B.I. Baseball 3 (USA) (Unl).nes */
	{ 0x2a01f9d1,  206, DEFAULT,    MI_V, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* Wagyan Land (Japan).nes */
	{ 0x7678f1d5,  207, DEFAULT, DFAULT8, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* Fudou Myouou Den (Japan).nes */
	{ 0x07eb2c12,  208, DEFAULT, DEFAULT, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* FC25-台湾格斗游戏+快打传说.nes */
	{ 0xdd8ced31,  209, DEFAULT, DEFAULT, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* Power Rangers III (Unl) [!].nes */
	{ 0x063b1151,  209, DEFAULT, DEFAULT, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* Power Rangers IV (Unl) [!].nes */
	{ 0xdd4d9a62,  209, DEFAULT, DEFAULT, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* Shin Samurai Spirits 2 - Haoumaru Jigoku Hen (Ch).nes */
	{ 0x0c47946d,  210,       1,    MI_V, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* Chibi Maruko-chan - Uki Uki Shopping (Japan).nes */
	{ 0xc247cc80,  210,       1,    MI_V, 1,    0x50, DEFAULT, DEFAULT, NOEXTRA }, /* Family Circuit '91 (Japan) (En).nes */
	{ 0x808606f0,  210,       1,    MI_V, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* Famista '91 (Japan).nes */
	{ 0x6ec51de5,  210,       2,    MI_H, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* Famista '92 (Japan).nes */
	{ 0xadffd64f,  210,       2,    MI_H, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* Famista '93 (Japan).nes */
	{ 0x429103c9,  210,       2,    MI_H, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* Famista '94 (Japan).nes */
	{ 0x81b7f1a8,  210,       1,    MI_V, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* Heisei Tensai Bakabon (Japan).nes */
	{ 0x2447e03b,  210,       2,    MI_H, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* Top Striker (Japan).nes */
	{ 0x1dc0f740,  210,       2,    MI_H, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* Wagyan Land 2 (Japan).nes */
	{ 0xd323b806,  210,       2,    MI_H, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* Wagyan Land 3 (Japan).nes */
	{ 0xbd523011,  210,       2,    MI_H, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* Namco Prism Zone - Dream Master (Japan).nes */
	{ 0x5daae69a,  211, DEFAULT, DEFAULT, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* Aladdin - Return of Jaffar, The (Unl) [!].nes */
	{ 0x1ec1dfeb,  217, DEFAULT, DEFAULT, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* 255-in-1 (Mapper 204) [p1].nes */
	{ 0x046d70cc,  217, DEFAULT, DEFAULT, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* ?? 500-in-1 (Anim Splash, Alt Mapper)[p1][!] */
	{ 0x12f86a4d,  217, DEFAULT, DEFAULT, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* 500-in-1.nes */
	{ 0xd09f778d,  217, DEFAULT, DEFAULT, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* ?? 9999999-in-1 (Static Splash, Alt Mapper)[p1][!] */
	{ 0x62ef6c79,  232, DEFAULT, DFAULT8, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* Quattro Sports (Camerica) (Aladdin) [b1].nes */
	{ 0x2705eaeb,  234, DEFAULT, DEFAULT, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* Maxi 15 (USA) (Unl).nes */
	{ 0x80cbcacb,  235, DEFAULT, DEFAULT, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* 100-in-1 (Unl).nes */
	{ 0x6175b9a0,  235, DEFAULT, DEFAULT, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* 150_in_1_199x-ASp.nes */
	{ 0x745a6791,  235,       2, DEFAULT, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* 210-in-1 and Contra 4-in-1 (212-in-1,212 Hong Kong,Reset Based)(Unl).nes */
	{ 0xdf81364d,  235, DEFAULT, DEFAULT, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* 260-in-1 [p1][!].nes */
	{ 0xa38f2f1d,  235, DEFAULT, DEFAULT, 0, DEFAULT,    0x07, DEFAULT, NOEXTRA }, /* 1500-in-1.nes */
	{ 0x6f12afc5,  235, DEFAULT, DEFAULT, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* ?? Golden Game 150-in-1 */
	{ 0x2537b3e6,  241, DEFAULT, DEFAULT, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* Dance Xtreme - Prima (Unl).nes */
	{ 0x11611e89,  241, DEFAULT, DEFAULT, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* Darkseed (Unl) [p1][b1].nes */
	{ 0x81a37827,  241, DEFAULT, DEFAULT, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* Darkseed (Unl) [p1].nes */
	{ 0xfb2b6b10,  241, DEFAULT, DEFAULT, 1, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* Fan Kong Jing Ying (Asia) (Unl).nes */
	{ 0xb5e83c9a,  241, DEFAULT, DEFAULT, 1, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* Xing Ji Zheng Ba (Asia) (Unl).nes */
	{ 0x368c19a8,  241, DEFAULT, DEFAULT, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* LIKO Study Cartridge 3-in-1 (Unl) [!].nes */
	{ 0x54d98b79,  241, DEFAULT, DEFAULT, 1, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* Titanic 1912 (Unl).nes */
	{ 0xc2730c30,   34, DEFAULT,    MI_H, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* Deadly Towers (USA).nes */
	{ 0xa21e675c,   34, DEFAULT,    MI_H, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* Mashou (Japan).nes */
	{ 0x6bea1235,  245, DEFAULT, DEFAULT, 1, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* Yong Zhe Dou E Long - Dragon Quest VI (Ch) [a1].nes */
	{ 0x345ee51a,  245, DEFAULT, DEFAULT, 1, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* Yong Zhe Dou E Long - Dragon Quest VII (Ch).nes */
	{ 0x57514c6c,  245, DEFAULT, DEFAULT, 1, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* Yong Zhe Dou E Long - Dragon Quest VI (Ch).nes */
	{ 0xdb9d71b7,  114, DEFAULT, DEFAULT, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* Super Donkey Kong (Unl) [o1].nes */
	
	/* KT-008 PCB. These require mapper 224 for having an outer bank register at $5xxx, but are usually found set to mapper 4. */
	{ 0x61fc4d20,  224, DEFAULT,    MI_H, 1,    0x70,    0x07,   DENDY, NOEXTRA }, /* (KT-1062) 口袋怪兽꞉ 水晶版.nes */
	{ 0xaa666c19,  224, DEFAULT,    MI_H, 1,    0x70,    0x07,   DENDY, NOEXTRA }, /* Ys Origin: Hugo.nes */
	{ 0xb0d011d3,  224, DEFAULT,    MI_H, 1,    0x70,    0x07,   DENDY, NOEXTRA }, /* Ys Origin: Yunica.nes */
	{ 0x16143319,  224, DEFAULT,    MI_H, 1,    0x70,    0x07,   DENDY, NOEXTRA }, /* Ys VI: 纳比斯汀的方舟.nes */
	{ 0xe05fc21f,  224, DEFAULT,    MI_H, 1,    0x70,    0x07,   DENDY, NOEXTRA }, /* Ys: 菲尔盖纳之誓约.nes */
	{ 0xf8b58b59,  224, DEFAULT,    MI_H, 1,    0x70,    0x07,   DENDY, NOEXTRA }, /* 三国志 - 蜀魏争霸.nes */
	{ 0xcb1bab3d,  224, DEFAULT,    MI_H, 1,    0x70,    0x07,   DENDY, NOEXTRA }, /* 三国志꞉ 蜀汉风云.nes */
	{ 0xddc122ed,  224, DEFAULT,    MI_H, 1,    0x70,    0x07,   DENDY, NOEXTRA }, /* 亡灵崛起.nes */
	{ 0x4d2811c7,  224, DEFAULT,    MI_H, 1,    0x70,    0x07,   DENDY, NOEXTRA }, /* 伏魔英雄传.nes */
	{ 0x24750e5d,  224, DEFAULT,    MI_H, 1,    0x70,    0x07,   DENDY, NOEXTRA }, /* 傲视天地.nes */
	{ 0xc658b6a8,  224, DEFAULT,    MI_H, 1,    0x70,    0x07,   DENDY, NOEXTRA }, /* 刀剑英雄传.nes */
	{ 0xea831217,  224, DEFAULT,    MI_H, 1,    0x70,    0x07,   DENDY, NOEXTRA }, /* 剑侠情缘.nes */
	{ 0x92ebad5b,  224, DEFAULT,    MI_H, 1,    0x70,    0x07,   DENDY, NOEXTRA }, /* 勇者斗恶龙 III꞉ 罪恶渊源.nes */
	{ 0x48210324,  224, DEFAULT,    MI_H, 1,    0x70,    0x07,   DENDY, NOEXTRA }, /* 勇者斗恶龙꞉ 勇者的试炼.nes */
	{ 0x3439d140,  224, DEFAULT,    MI_H, 1,    0x70,    0x07,   DENDY, NOEXTRA }, /* 勇者斗恶龙꞉ 天空的新娘.nes */
	{ 0x9a0a531a,  224, DEFAULT,    MI_H, 1,    0x70,    0x07,   DENDY, NOEXTRA }, /* 勇者斗恶龙꞉ 天空篇.nes */
	{ 0xb5fdb3cb,  224, DEFAULT,    MI_H, 1,    0x70,    0x07,   DENDY, NOEXTRA }, /* 勇者黑暗世界 - 混沌世界.nes */
	{ 0x4f427110,  224, DEFAULT,    MI_H, 1,    0x70,    0x07,   DENDY, NOEXTRA }, /* 口袋怪兽꞉ 珍珠版.nes */
	{ 0xa46d6f4c,  224, DEFAULT,    MI_H, 1,    0x70,    0x07,   DENDY, NOEXTRA }, /* 口袋怪兽꞉ 琥珀版.nes */
	{ 0xe001de16,  224, DEFAULT,    MI_H, 1,    0x70,    0x07,   DENDY, NOEXTRA }, /* 口袋怪兽꞉ 白金版.nes */
	{ 0x5464d7f8,  224, DEFAULT,    MI_H, 1,    0x70,    0x07,   DENDY, NOEXTRA }, /* 口袋怪兽꞉ 翡翠版.nes */
	{ 0x4d735cb1,  224, DEFAULT,    MI_H, 1,    0x70,    0x07,   DENDY, NOEXTRA }, /* 口袋怪兽꞉ 钻石版.nes */
	{ 0x5d04547c,  224, DEFAULT,    MI_H, 1,    0x70,    0x07,   DENDY, NOEXTRA }, /* 失落的神器.nes */
	{ 0xcb524b42,  224, DEFAULT,    MI_H, 1,    0x70,    0x07,   DENDY, NOEXTRA }, /* 征战天下.nes */
	{ 0x5f362198,  224, DEFAULT,    MI_H, 1,    0x70,    0x07,   DENDY, NOEXTRA }, /* 战神世界.nes */
	{ 0x36de88e7,  224, DEFAULT,    MI_H, 1,    0x70,    0x07,   DENDY, NOEXTRA }, /* 新魔界.nes */
	{ 0xf8e9c9cf,  224, DEFAULT,    MI_H, 1,    0x70,    0x07,   DENDY, NOEXTRA }, /* 无双乱舞.nes */
	{ 0xa4c39535,  224, DEFAULT,    MI_H, 1,    0x70,    0x07,   DENDY, NOEXTRA }, /* 神魔大陆.nes */
	{ 0x76bbe916,  224, DEFAULT,    MI_H, 1,    0x70,    0x07,   DENDY, NOEXTRA }, /* 落日征战.nes */
	{ 0x9b518d54,  224, DEFAULT,    MI_H, 1,    0x70,    0x07,   DENDY, NOEXTRA }, /* 轩辕剑꞉ 云的彼端.nes */
	{ 0x91396b3f,  224, DEFAULT,    MI_H, 1,    0x70,    0x07,   DENDY, NOEXTRA }, /* 轩辕剑꞉ 天之痕.nes */
	{ 0xaa621fa0,  224, DEFAULT,    MI_H, 1,    0x70,    0x07,   DENDY, NOEXTRA }, /* 轩辕剑꞉ 枫之舞.nes */
	{ 0x48d1f54a,  224, DEFAULT,    MI_H, 1,    0x70,    0x07,   DENDY, NOEXTRA }, /* 轩辕剑꞉ 王者归来.nes */
	
	{ 0xbdbe3c96,  238, DEFAULT,    MI_V, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* Contra Fighter (Unl).nes */
	{ 0xcb53c523,   11, DEFAULT,    MI_V, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* King Neptune's Adventure (USA) (Unl).nes */
	{ 0x6e149729,  189, DEFAULT, DEFAULT, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* Master Fighter II (Unl) [a1].nes */
	{ 0x60bfeb0c,   90, DEFAULT, DEFAULT, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* Mortal Kombat 2 (Unl) [!].nes */
	{ 0x247cc73d,  150, DEFAULT, DEFAULT, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* Poker II (Asia) (Ja) (Unl).nes */
	{ 0x1f1326d4,  121, DEFAULT, DEFAULT, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* Super Sonic 5 (1997) (Unl) [!].nes */
	{ 0x99748230,  215, DEFAULT, DEFAULT, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* EarthWorm Jim 2 (SuperGame).nes */
	{ 0x37876ac7,  215, DEFAULT, DEFAULT, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* Golden Card 6-in-1 (Unl) [!].nes */
	{ 0x1a3320a3,  215, DEFAULT, DEFAULT, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* Mortal Kombat 3 (SuperGame).nes */
	{ 0xec70f8d8,  258, DEFAULT, DEFAULT, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* 1997 留念: 港-京 - Super Hang-On (protected version, CHR doubled) */
	{ 0x224989d9,  258, DEFAULT, DEFAULT, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* 1997 留念: 港-京 - Super Hang-On (protected version) */
	{ 0x80eb1839,  114,       1, DEFAULT, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* Boogerman (Sugar Softec) (Unl) [!].nes */
	{ 0x071e4ee8,  114, DEFAULT, DEFAULT, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* ?? m114,submapper 1 test rom */
	{ 0xfe3e03a1,  197, DEFAULT, DEFAULT, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* Mortal Kombat III Special (YY-030) (Ch) [!].nes */
	{ 0x9151d311,  197, DEFAULT, DEFAULT, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* Mortal Kombat III 28 Peoples (NT-328) (Ch) [!].nes */
	{ 0x272709b9,  237, DEFAULT, DEFAULT, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* Teletubbies Y2K (420-in-1).nes */
	{ 0x2e27e0af,  227, DEFAULT, DEFAULT, 1,    0x70,    0x07,   DENDY, NOEXTRA }, /* Bio Hazard.nes */
	{ 0x0e7e9309,  189, DEFAULT, DEFAULT, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* Street Fighter II - The World Warrior (Unl) [a1].nes */
	{ 0xa3ac0095,  189, DEFAULT, DEFAULT, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* Street Fighter II - The World Warrior (Unl) [a2].nes */
	{ 0xeced5899,  121, DEFAULT, DEFAULT, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* Ultimate Mortal Kombat 4 (Unl) [!].nes */
	{ 0x19c1ed51,  150, DEFAULT, DEFAULT, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* Poker III (Asia) (Ja) (Unl).nes */
	{ 0x282745c5,  141, DEFAULT, DEFAULT, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* Q Boy (Asia) (Ja) (Unl).nes */
	{ 0x4b9ecfb2,   21, DEFAULT, DEFAULT, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* Wai Wai World 2 - SOS!! Paseri Jou (Japan) (Virtual Console).nes */
	{ 0x33751782,    4, DEFAULT, DEFAULT, 1, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* Zoda's Revenge - StarTropics II (USA, Europe) (Virtual Console).nes */
	{ 0x9bbf3e5d,   15, DEFAULT, DEFAULT, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* 168-in-1 [p1][!].nes */

	/* TXC / Sachen / JoyVan */

	/* This cart does not work anymore with latest mapper implementation.
	 * This will be run using alternate mapper using override in m132 */
	{ 0x2a5f4c5a,  132, DEFAULT, DEFAULT, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* Zhan Guo Si Chuan Sheng (C&E) (Unl).nes */

	{ 0x0acfc3cd,  132, DEFAULT, DEFAULT, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* Mahjong Block (MGC-008) (Unl) [!].nes */

	/* ines mappers that uses iNes 2.0 numbers */

	/* CRC32, mapperNum, SubMapper, Mirroring, hasBattery, prgRam, chrRam, region, extra flags */
	{ 0xf6bd8e31,  281, DEFAULT,    MI_H, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* 1997 Super HIK 4-in-1 (JY-052) [p1][!].nes */
	/* dumps are assigned 281, but its 293 on nesdev */
	{ 0x06256C80,  293, DEFAULT, DEFAULT, 0, DEFAULT,    0x07, DEFAULT, NOEXTRA }, /* Super 12-in-1 NewStar (UNL) */
	{ 0x5aa23a15,  361, DEFAULT,    MI_H, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* 4-in-1 (OK-411)[p1][!].nes */
	{ 0xf6b9d088,  366, DEFAULT,    MI_H, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* 4-in-1 (K-3131GS, GN-45) [p1][!].nes */
	{ 0x503566b2,  366, DEFAULT,    MI_H, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* 4-in-1 (K-3131SS, GN-45) [p1][!].nes */
	{ 0xdb2d2d88,  369, DEFAULT, DEFAULT, 0,    0x07, DEFAULT, DEFAULT, NOEXTRA }, /* Super Mario Bros. Party.nes */
	{ 0x87f83ea2,  380, DEFAULT, DEFAULT, 0, DEFAULT,    0x07, DEFAULT, NOEXTRA }, /* 42 to 80,000 */
	{ 0xc4b94bd5,  389, DEFAULT, DEFAULT, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* Caltron - 9 in 1 (USA) (Proto) (Unl).nes */
	{ 0x2F497313,  401, DEFAULT, DEFAULT, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* Super 19-in-1 (VIP19) */

	/* ines mappers that uses unif boards */

	{ 0x0073dbd8,  260, DEFAULT, DEFAULT, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* 2-in-1 - Mortal Kombat V Turbo 30 + Super Aladdin (Unl) [p1][!].nes */
	{ 0x4dc6107d,  260, DEFAULT, DEFAULT, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* 2-in-1 - Boogerman + Flintstones, The (Unl) [p1][!].nes */
	{ 0xb72b2cf4,  260, DEFAULT, DEFAULT, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* 2-in-1 - Aladdin + Lion King, The (Unl) [p1][!].nes */
	{ 0x5638ba59,  260, DEFAULT, DEFAULT, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* Mortal Kombat Trilogy - 8 People (M1274) (Ch) [!].nes */
	{ 0xa1dc16c0,  262, DEFAULT, DEFAULT, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* Street Heroes (Asia) (Ja) (Unl).nes */
	{ 0x1df10182,  263, DEFAULT, DEFAULT, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* Boogerman II (Rex-Soft) [!].nes */
	{ 0xf956fcea,  521, DEFAULT, DEFAULT, 0, DEFAULT,    0x70, DEFAULT, NOEXTRA }, /* Korean Igo (Korea) (Unl).nes */
	{ 0x2eed2e34,  289, DEFAULT, DEFAULT, 0, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }, /* 76-in-1 [p1][a1].nes 2048 PRG, 0 CHR */

	{ 0x00000000, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, NOEXTRA }

#endif
