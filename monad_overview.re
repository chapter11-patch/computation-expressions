= モナド概論

//lead{
本章では、モナドとコンピュテーション式の概要と関連性について解説します。
//}

== モナド

型が存在するプログラミング言語では、何らかの状態を表したり、性質を持つことを型で表現します、もとい表現すべきです。
以降では"何らかの状態を表す型"や"何らかの性質を持つ型"という冗長な表現は避け、@<kw>{文脈}と呼ぶことにします。
ここではある文脈を、型パラメータを一つ取る型 M を使って @<code>{M<'T>} と表現することにしましょう。
@<code>{'T} は任意の型です。

@<code>{'T} は何かしらの手順 ── 手続きを経なければ、 @<code>{M<'T>} に変換できません。
@<code>{'T} を @<code>{M<'T>} に変換する関数は、次のようになります。

//emlist{
// return 関数は 'T を 文脈 M に持ち上げる
let return : 'T -> M<'T> = ...
//}

以降では、この変換を "持ち上げる" と表現します。

次に、 @<code>{'T} に計算を適用しつつ 文脈 M に持ち上げる関数を考えてみましょう。

//emlist{
// 'T に計算を適用した結果 'U を持ち上げる関数の型
'T -> M<'U>
//}

仮に、この関数に適用できそうな値が @<code>{M<'T>} しかない場合はどうすればよいでしょう？
似たような関数 @<code>{M<'T> -> M<'U>} を定義すれば問題は解決できますが、これでは似たような計算を何度も定義することになってしまいます。

このような重複を避けるには、文脈から @<code>{'T} を取り出しつつ、計算結果を再び文脈 M に持ち上げて返す手続きがあればよさそうです。

//emlist{
// M<'T> から 'T を取り出し計算を適用、計算結果は再び文脈に持ち上げる。
let (>>=) : M<'T> -> ('T -> M<'U>) -> M<'U> = ...
//}

この手続きは bind とも呼ばれます。

各文脈に対して return と bind を定義できれば、次のような計算が記述できるようになります。

//emlist{
// f1, f2... は何らかの計算
f1 x0 >>= fun x1 ->
f2 x1 >>= fun x2 ->
...
fn xm >>= fun xn -> return (g xn)
//}

この記述は、 return と bind を持つ任意の文脈に適用できます。
つまり、 return と bind を導入することで、計算を同じ方法で記述ができるようになったのです。

ここで、 return と bind を使ったいくつかの計算を見てみましょう。
一つ目は、return の 後に bind を適用する計算です。

//emlist{
// x を持ち上げた文脈から値を取り出し、計算結果を再度持ち上げる
(return x) >>= f
//}

この計算は、 return で持ち上げた値 x を bind ですぐに取り出し、 f に適用しています。
持ち上げた直後に取り出す計算は冗長なので、なくすことができると嬉しいですね。

//emlist{
// (return x) >>= f は f x に置き換えできる
(return x) >>= f = f x
//}

二つ目は、bind に return を適用した計算です。

//emlist{
// 文脈から値を取り出し、持ち上げる
m >>= return
//}

この計算は、文脈 m から取り出した値を同じ文脈に持ち上げなおしているだけです。
つまり、m から変化がないのが自然です。

//emlist{
m >>= return = m
//}

最後は、 bind がネストしている計算です。

//emlist{
m >>= (fun x -> f x >>= g)
//}

この計算はラムダ式によって多段構造になっていますが、f を m 側の bind に適用できれば、平坦化可能になります。

//emlist{
// 計算が平坦化できる
m >>= f >>= g = m >>= (fun x -> f x >>= g)
//}

これら三つの変換法則を満たす return と bind が定義された文脈 M であれば、 return と bind を用いて計算を抽象化できます。
言い換えると、手続きを一般化できたことになります。
この、特定の関数を用いて手続きを一般化できる文脈 M はモナドと呼ばれています。
また、三つの変換規則はそれぞれ右恒等性、左恒等性、結合則と呼ばれており、この三法則をあわせてモナド則と呼びます。

