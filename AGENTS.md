# keyball (shishi のフォーク)

[Yowkees/keyball](https://github.com/Yowkees/keyball) のフォークで、shishi が使うキーマップを管理するリポジトリです。公開リポジトリなので、個人のマシン名・私的なパスや接続先・認証情報は書き込みません。

## ブランチ

- `shishi` が既定ブランチです。キーマップや CI の変更はここへ入れます。
- upstream (Yowkees/keyball) の変更は、`shishi` へ merge して取り込んでいます。

## 対象のキーマップ

- 「shishi のキーマップ」「shishi プロファイル」は、keyball39 の `qmk_firmware/keyboards/keyball/keyball39/keymaps/shishi/` を指します。機種を問い直さずに keyball39 を変更します。
- keyball61 にも `keymaps/shishi/` がありますが、変更するのは keyball61 と明示されたときだけです。keyball39 への変更を keyball61 へ写すのも、明示されたときだけにします。
- keymap は 3 ファイルです。`keymap.c` にはレイヤー・tap dance・combo・キーごとの tapping term があります。`config.h` には全体の tapping term などの定義があり、`rules.mk` では使う QMK の機能を有効にします。

## タップとホールドの時間

- 全体の値は `config.h` の `TAPPING_TERM` です。キーごとの値は `keymap.c` の `get_tapping_term()` が返します。
- QMK が `get_tapping_term()` を呼ぶのは、`TAPPING_TERM_PER_KEY` が定義されているときだけです (qmk 0.22.14 の `quantum/action_tapping.h`)。この keymap では `config.h` で定義しています。定義が無くてもビルドは通り、関数は最適化で消えて、全キーが `TAPPING_TERM` で動きます。この定義は残します。

## ビルドと確認

- `.github/workflows/build-shishi.yml` の `paths` に当たるファイル (keymap のソースや workflow) を変えて push すると、GitHub Actions の「Build own firmwares」が keyball39 と keyball61 の shishi をビルドします。文書だけを変えた push ではビルドしません。
- hex は `keyball39-shishi-firmware` などの artifact に入り、`gh run download <run id> -R shishi/keyball -n keyball39-shishi-firmware` で取得できます。
- 「Build all firmwares」(`build-all.yml`) は upstream の default・test・via などの keymap をビルドします。shishi の keymap は含みません。
- ATmega32U4 に書き込めるのは 28672 バイトまでで、keyball39 の shishi は上限近くまで使っています。機能を足したら、「Check size」ジョブの出力で上限内に収まっているかを確かめます。
- 数値やキーコードを変えたら、ビルドしたファームウェアにその変更が入っているかを確かめます。`TAPPING_TERM_PER_KEY` の例のように、呼ばれないコードはエラーにならずに消えるためです。
- キーボードへの書き込みと打ち心地の確認は、利用者が実機で行います。

### 手元で CI と同じ条件でビルドする

CI と同じ版の qmk と、`.github/workflows/build-firmware.yml` で digest まで固定した `ghcr.io/qmk/qmk_cli` イメージを使います。qmk の版は `.github/actions/checkout-qmk_firmware/action.yml` の `version` の既定値です。`build-firmware.yml` はこの action に版を渡さないため、workflow 側の `qmk_version` はビルドに使われません。ほかの版では生成物が変わるため、確認には使いません。

1. qmk_firmware をその版のタグで、submodule ごと clone します。`<qmk の版>` には `version` の既定値を入れます。

   ```sh
   git clone --depth 1 --branch <qmk の版> --recurse-submodules --shallow-submodules https://github.com/qmk/qmk_firmware.git <qmk の clone>
   ```

2. clone とこの repo の `qmk_firmware/keyboards/keyball` を mount してビルドします。`<qmk の clone>` と `<この repo>` には絶対パスを入れます。`<イメージ>` には `build-firmware.yml` の `container.image` の値を入れます。

   ```sh
   docker run --rm -e OWNER="$(id -u):$(id -g)" -v <qmk の clone>:/qmk -v <この repo>/qmk_firmware/keyboards/keyball:/src/keyball:ro -w /qmk <イメージ> bash -c 'set -e; cleanup() { rm -f keyboards/keyball; chown -R "$OWNER" /qmk; }; trap cleanup EXIT; rm -rf .build keyball_keyball39_shishi.hex disasm.txt; /usr/bin/python3 -m pip install -q -r requirements.txt; qmk config user.qmk_home=/qmk; ln -sfn /src/keyball keyboards/keyball; qmk compile -j 4 -kb keyball/keyball39 -km shishi; avr-objdump -d .build/keyball_keyball39_shishi.elf > disasm.txt'
   ```

   clone の直下に hex (`keyball_keyball39_shishi.hex`) と逆アセンブル結果 (`disasm.txt`) ができます。

- ビルドのたびに `.build` と前回の hex・`disasm.txt` を消します。別の checkout に向き先を替えてビルドすると、ソースの更新時刻が前回のオブジェクトより古いことがあります。すると make が前回のオブジェクトを再利用し、変更前と同じ hex ができます。
- コンテナは root でファイルを書きます。終了時の `cleanup` が一時リンクを消し、clone の所有者を戻します。ビルドに失敗したときも実行します。
- 変更が入ったかは、変更前と変更後の `disasm.txt` を比べて確かめます。上のコマンドは前回の `disasm.txt` を消すため、変更前の状態でビルドしてできた `disasm.txt` は、変更後をビルドする前に別名 (例: `disasm.before.txt`) へ移します。16 ビットの値は下位と上位のバイトに分かれて即値に現れます (例: 200 は `0xC8` と `0x00`)。上位が 0 のときは常に 0 のレジスター (`r1`) が使われ、即値に現れないことがあります。keymap 配列のキーコードは命令ではなくデータとして入るため、即値ではなく差分の箇所で確かめます。
- hex のサイズは、`build-shishi.yml` の「List size of firmwares」と同じコマンドで計算できます。
