../KiCad_Project/msx0_keyboard の基板用ファームウェアです。
ファームウェアは二種類あります。

	(1) FaceII_keyboard_compatible
		こちらの通信プロトコルは、FaceIIキーボードのプロトコルを使うバージョンです。
		現時点で利用可能である反面、プロトコルの都合によりいくつかのキーを
		入力できない問題があります。

	(2) NewProtocol_MSX0_keyboard  ★こちらがやりたい方！
		こちらの通信プロトコルは、新プロトコルになっております。
		MSX0側が対応していただかなければ、このファームウェアで入力できません。
		対応していただければ、実MSXと同じキー入力を実現できます。

　夫々のフォルダに置いてある uf2 ファイルがコンパイル済みファームウェアです。
